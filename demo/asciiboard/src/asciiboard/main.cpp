#include <asciiboard/asciiboard.hpp>
#include <asciiboard/demo_midi_generator.hpp>
#include <asciiboard/render_loop.hpp>

#include <common/exception.hpp>
#include <common/log.hpp>
#include <io/audio_output_stream.hpp>
#include <io/midi_input_stream.hpp>
#include <synth/synthesizer_processor.hpp>
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

        // Initial GUI / synthesizer values
        auto initialControls = asciiboard::SynthControls{synth::ADSR{.01, .1, .8, .01}, .8, 0, .2};

        // These wave tables are sampled by the synthesizers oscillators.
        auto weightedWaveTables = std::vector{
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::sineWaveFill), initialControls.sineWeight},
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::squareWaveFill), initialControls.squareWeight},
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::triangleWaveFill), initialControls.triangleWeight}};


        // The midi thread is created and started.
        auto midiHandle = std::make_shared<common::midi::MidiHandle>();
        auto midiInputStream = io::MidiInputStream(midiHandle);
        trySelectPort(midiInputStream);

        // If midi isn't available, a demo midi generator is used.
        auto midiGenerator = asciiboard::demo::MidiGenerator(midiHandle);
        if (midiInputStream.isOpen()) {
            midiInputStream.start();
        } else {
            midiGenerator.start();
        }

        // The audio output thread is created and started.
        auto outputBufferHandle = std::make_shared<common::audio::RingBuffer<>>();
        auto audioOutputStream = io::AudioOutputStream{outputBufferHandle};
        if (audioOutputStream.createStreamError() != io::AudioStreamError::NoError) {
            throw common::MicrotoneException("Failed to create audio output stream.");
        }
        audioOutputStream.start();

        // The synthesizer thread is created and started.
        // TODO: Caller should create voices.
        auto synth = std::make_shared<synth::Synthesizer>(audioOutputStream.sampleRate(), weightedWaveTables);
        // This must remain alive throughout the lifetime of this application!
        auto synthesizerProcess = synth::SynthesizerProcessor{synth, midiHandle, outputBufferHandle};
        synthesizerProcess.start();

        auto renderLoop = asciiboard::RenderLoop{asciiboard, midiHandle, outputBufferHandle};
        renderLoop.start();

        auto envelope = synth::Envelope(initialControls.adsr, audioOutputStream.sampleRate());

        // Callback invoked when asciiboard controls are changed
        auto onControlsChangedFn = [&synth, &weightedWaveTables, &envelope](const asciiboard::SynthControls& controls) {
            auto shouldUpdateWaveTables = false;
            if (controls.sineWeight != weightedWaveTables[0].weight) {
                weightedWaveTables[0].weight = controls.sineWeight;
                shouldUpdateWaveTables = true;
            }
            if (controls.squareWeight != weightedWaveTables[1].weight) {
                weightedWaveTables[1].weight = controls.squareWeight;
                shouldUpdateWaveTables = true;
            }
            if (controls.triangleWeight != weightedWaveTables[2].weight) {
                weightedWaveTables[2].weight = controls.triangleWeight;
                shouldUpdateWaveTables = true;
            }
            if (shouldUpdateWaveTables) {
                synth->setWaveTables(weightedWaveTables);
            }

            if (controls.adsr != envelope.adsr()) {
                envelope.setAdsr(controls.adsr);
                synth->setEnvelope(envelope);
            }
        };

        // Log statistics before exiting.
        auto onAboutToQuitFn = [&outputBufferHandle]() {
            auto stats = outputBufferHandle->getStatistics();
            M_INFO(fmt::format("Processed {} audio blocks. Dropped {} blocks.", stats.numBlocksPopped, stats.numBlocksDropped));
            common::Log::shutdown();
        };

        // Blocks this thread
        asciiboard->loop(initialControls, onControlsChangedFn, onAboutToQuitFn);

    } catch (common::MicrotoneException& e) {
        std::cout << fmt::format("Microtone error: {}", e.what()) << std::endl;
    }

    return 0;
}
