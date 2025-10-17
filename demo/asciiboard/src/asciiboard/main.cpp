#include <asciiboard/asciiboard.hpp>

#include <common/exception.hpp>
#include <common/log.hpp>
#include <io/midi_input.hpp>
#include <synth/audio_buffer.hpp>
#include <synth/synthesizer.hpp>
#include <synth/wave_table.hpp>

#include <fmt/format.h>

#include <iostream>
#include <string>

namespace {

//! Reads a port from standard in.
[[nodiscard]] int getUserMidiPortPreference(const io::MidiInput& midiInput) {
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
void trySelectPort(io::MidiInput& midiInput) {
    switch (midiInput.portCount()) {
    case 0:
        std::cout << "No midi ports are available. Continuing without midi..." << std::endl;
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

    try {
        // Asciiboard
        auto asciiboard = asciiboard::Asciiboard();

        // Initial GUI / synthesizer values
        auto initialControls = asciiboard::SynthControls{synth::ADSR{.01, .1, .8, .01}, .8, 0, .2};

        // These wave tables are sampled by the synthesizers oscillators.
        auto weightedWaveTables = std::vector{
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::sineWaveFill), initialControls.sineWeight},
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::squareWaveFill), initialControls.squareWeight},
            synth::WeightedWaveTable{synth::buildWaveTable(synth::examples::triangleWaveFill), initialControls.triangleWeight}
        };

        // This callback is invoked on every audio frame. Don't do anything blocking here!
        auto onOutputFn = [&asciiboard](const synth::AudioBuffer& outputData) {
            asciiboard.addOutputData(outputData);
        };

        // Create synthesizer
        auto synth = synth::Synthesizer{weightedWaveTables, onOutputFn};

        // Listen to midi input
        auto midiInput = io::MidiInput();
        trySelectPort(midiInput);

        if (midiInput.isOpen()) {
            // Start midi input, update UI and synth when midi data changes
            midiInput.start([&synth, &asciiboard](int status, int note, int velocity) {
                synth.addMidiData(status, note, velocity);
                asciiboard.addMidiData(status, note, velocity);
            });
        }

        auto envelope = synth::Envelope(initialControls.adsr, synth.sampleRate());

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
                synth.setWaveTables(weightedWaveTables);
            }

            if (controls.adsr != envelope.adsr()) {
                envelope.setAdsr(controls.adsr);
                synth.setEnvelope(envelope);
            }
        };
        synth.start();

        // Blocks this thread
        asciiboard.loop(initialControls, onControlsChangedFn);

    } catch (common::MicrotoneException& e) {
        std::cout << fmt::format("Microtone error: {}", e.what()) << std::endl;
    }

    return 0;
}
