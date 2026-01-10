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
    common::Log::init(/* enableConsoleLogging= */ false);
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

        // GUI / synthesizer controls
        auto controls = asciiboard::SynthControls{
            .attack_pct = 1,
            .decay_pct = 10,
            .sustain_pct = 80,
            .release_pct = 1,
            .sineWeight_pct = 80,
            .squareWeight_pct = 0,
            .triangleWeight_pct = 20,
            .gain = 0.9f,
            .lfoFrequency_Hz = 0.25f,
            .lfoGain = 0.1f,
            .delay_ms = 180.f,
            .delayGain = 0.4f,
        };

        // Audio input (source)
        auto synth = std::make_shared<synth::Synthesizer>(
            audioOutputStream.sampleRate(),
            synth::TripleWaveTableT{
                .waveTables = {
                    synth::buildWaveTable(synth::examples::sineWaveFill),
                    synth::buildWaveTable(synth::examples::squareWaveFill),
                    synth::buildWaveTable(synth::examples::triangleWaveFill)},
                .weights = controls.getOscillatorWeights()},
            controls.gain,
            controls.getAdsr(),
            controls.lfoFrequency_Hz,
            controls.lfoGain);

        // Effects
        auto delay = std::make_shared<synth::Delay>(controls.getDelay_samples(audioOutputStream.sampleRate()), controls.delayGain);

        // Audio output (sink)
        auto outputDevice = std::make_shared<synth::OutputDevice>(outputBufferHandle);

        // The audio pipeline of the instrument.
        auto audioPipeline = synth::AudioPipeline{
            synth,
            {delay},
            outputDevice};

        // The thread responsible for running our audio pipeline.
        auto instrument = synth::Instrument{midiHandle, std::move(audioPipeline)};
        instrument.start();

        // Start audio output after everything else:
        audioOutputStream.start();

        auto renderLoop = asciiboard::RenderLoop{asciiboard, midiHandle, synth};
        renderLoop.start();

        auto onControlsChangedFn = [&](const asciiboard::SynthControls& newControls) {
            controls.applyChanges(*synth, newControls);
            controls.applyChanges(*delay, newControls, audioOutputStream.sampleRate());
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
