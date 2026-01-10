#include <asciiboard/asciiboard.hpp>
#include <asciiboard/demo_midi_generator.hpp>
#include <asciiboard/render_loop.hpp>

#include <common/exception.hpp>
#include <common/log.hpp>
#include <io/audio_output_stream.hpp>
#include <io/midi_input_stream.hpp>
#include <synth/effects/delay.hpp>
#include <synth/instrument.hpp>
#include <synth/wave_table.hpp>

#include <fmt/format.h>

#include <iostream>
#include <string>

namespace {

//! Reads a port from standard in.
[[nodiscard]] int getUserMidiPortPreference(const io::MidiInputStream& midiInput) {
    int selectedPort = -1;
    while (selectedPort < 0 || selectedPort >= midiInput.portCount()) {
        std::cout << fmt::format("Please choose a port between 0 and {}.", midiInput.portCount() - 1) << std::endl;
        for (auto i = 0; i < midiInput.portCount(); ++i) {
            std::cout << fmt::format("{0}. {1}", i, midiInput.portName(i)) << std::endl;
        }

        auto input = std::string{};
        std::getline(std::cin, input);
        try {
            selectedPort = std::stoi(input);
        } catch (...) {
            std::cout << "Enter a number." << std::endl;
        }
    }
    return selectedPort;
}

//! Selects a midi port automatically, if possible. Awaits user input if there are multiple ports available.
void trySelectPort(io::MidiInputStream& midiInput) {
    switch (midiInput.portCount()) {
    case 0:
        std::cout << "No midi ports are available. Continuing with generated midi..." << std::endl;
        break;
    case 1:
        std::cout << "Using midi port 0 (the only one available)." << std::endl;
        midiInput.openPort(0);
        break;
    default:
        auto userPreference = getUserMidiPortPreference(midiInput);
        midiInput.openPort(userPreference);
        break;
    }
}

}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    common::Log::init();
    M_INFO(fmt::format("Started logging: {}", common::Log::getDefaultLogfilePath()));

    try {
        // Asciiboard
        auto asciiboard = std::make_shared<asciiboard::Asciiboard>();

        // The audio output thread is created and started.
        auto outputBufferHandle = std::make_shared<common::RingBuffer<common::audio::FrameBlock>>();
        auto audioOutputStream = io::AudioOutputStream{outputBufferHandle};
        if (audioOutputStream.createStreamError() != io::AudioStreamError::NoError) {
            throw common::MicrotoneException("Failed to create audio output stream.");
        }

        // The midi thread is created and started.
        auto midiHandle = std::make_shared<common::midi::TwoReaderMidiHandle>();
        auto midiInputStream = io::MidiInputStream(midiHandle);
        trySelectPort(midiInputStream);

        // If midi isn't available, a demo midi generator is used.
        auto midiGenerator = asciiboard::demo::MidiGenerator(midiHandle);
        if (midiInputStream.isOpen()) {
            midiInputStream.start();
        } else {
            midiGenerator.start();
        }

        // Initial GUI / synthesizer values
        auto initialAttack = 1;
        auto initialDecay = 10;
        auto initialSustain = 80;
        auto initialRelease = 1;
        auto initialGain = .9f;
        auto initialLfoFrequencyHz = .25f;
        auto initialLfoGain = .1f;

        auto initialDelay_ms = 180.f;
        auto initialDelayGain = 0.4f;
        auto controls = asciiboard::SynthControls{
            initialAttack,
            initialDecay,
            initialSustain,
            initialRelease,
            80,
            0,
            20,
            initialGain,
            initialLfoFrequencyHz,
            initialLfoGain,
            initialDelay_ms,
            initialDelayGain,
        };

        // These wave tables are sampled by the synthesizers oscillators.
        auto weightedWaveTables = std::vector{
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::sineWaveFill), controls.sineWeight / 100.},
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::squareWaveFill), controls.squareWeight / 100.},
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::triangleWaveFill), controls.triangleWeight / 100.}};

        // ADSR of the synthesizer.
        auto envelope = synth::Envelope(controls.getAdsr(), audioOutputStream.sampleRate());

        // The synthesizer thread is created and started.
        auto synth = std::make_shared<synth::Synthesizer>(
            audioOutputStream.sampleRate(),
            weightedWaveTables,
            initialGain,
            controls.getAdsr(),
            initialLfoFrequencyHz,
            initialLfoGain);

        // For now this is a simple audio output device, but it'll soon record into a memory buffer.
        auto outputDevice = std::make_shared<synth::OutputDevice>(outputBufferHandle);

        // Effects
        auto delay = std::make_shared<synth::Delay>(controls.delay_ms / 1000 * audioOutputStream.sampleRate(), controls.delayGain);

        // The audio pipeline of the instrument.
        auto audioPipeline = synth::AudioPipeline{
            synth,
            {delay},
            outputDevice
        };

        // The thread responsible for running our audio pipeline.
        auto instrument = synth::Instrument{midiHandle, std::move(audioPipeline)};
        instrument.start();

        // Start audio output after everything else:
        audioOutputStream.start();

        auto renderLoop = asciiboard::RenderLoop{asciiboard, midiHandle, synth};
        renderLoop.start();

        auto onControlsChangedFn = [&synth, &controls, &weightedWaveTables, &envelope](const asciiboard::SynthControls& newControls) {
            if (controls.sineWeight != newControls.sineWeight || controls.squareWeight != newControls.squareWeight || controls.triangleWeight != newControls.triangleWeight) {
                weightedWaveTables[0].weight = controls.sineWeight;
                weightedWaveTables[1].weight = controls.squareWeight;
                weightedWaveTables[2].weight = controls.triangleWeight;

                //! TODO: setWeights.
                synth->setWaveTables(weightedWaveTables);
            }

            if (controls.gain != newControls.gain) {
                synth->setGain(controls.gain);
            }

            if (controls.lfoFrequencyHz != newControls.lfoFrequencyHz) {
                synth->setLfoFrequency(controls.lfoFrequencyHz);
            }

            if (controls.lfoGain != newControls.lfoGain) {
                synth->setLfoGain(controls.lfoGain);
            }

            if (controls.getAdsr() != newControls.getAdsr()) {
                envelope.setAdsr(controls.getAdsr());
                synth->setEnvelope(envelope);
            }

            controls = newControls;
        };

        auto onAboutToQuitFn = []() {
            common::Log::shutdown();
        };

        // Blocks this thread
        asciiboard->loop(controls, onControlsChangedFn, onAboutToQuitFn);

    } catch (common::MicrotoneException& e) {
        std::cout << fmt::format("Microtone error: {}", e.what()) << std::endl;
    }

    return 0;
}
