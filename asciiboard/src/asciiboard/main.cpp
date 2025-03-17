#include <asciiboard/asciiboard.hpp>

#include <microtone/exception.hpp>
#include <microtone/midi_input.hpp>
#include <microtone/synthesizer/audio_buffer.hpp>
#include <microtone/synthesizer/synthesizer.hpp>
#include <microtone/synthesizer/weighted_wavetable.hpp>

#include <fmt/format.h>

#include <iostream>
#include <string>

namespace {

//! Reads a port from standard in.
[[nodiscard]] int getUserMidiPortPreference(const microtone::MidiInput& midiInput) {
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
void trySelectPort(microtone::MidiInput& midiInput) {
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
    microtone::Platform::init();

    try {
        // Asciiboard
        auto asciiboard = asciiboard::Asciiboard();

        // Initial GUI / synthesizer values
        auto initialControls = asciiboard::SynthControls{};
        initialControls.sineWeight = 0.8;
        initialControls.squareWeight = 0;
        initialControls.triangleWeight = 0.2;

        // These wave tables are sampled by the synthesizers oscillators.
        auto weightedWaveTables = std::vector<microtone::WeightedWaveTable>{};

        auto sineWaveTable = microtone::buildWaveTable(microtone::examples::sineWaveFill);
        auto squareWaveTable = microtone::buildWaveTable(microtone::examples::squareWaveFill);
        auto triangleWaveTable = microtone::buildWaveTable(microtone::examples::triangleWaveFill);

        weightedWaveTables.emplace_back(sineWaveTable, initialControls.sineWeight);
        weightedWaveTables.emplace_back(squareWaveTable, initialControls.squareWeight);
        weightedWaveTables.emplace_back(triangleWaveTable, initialControls.triangleWeight);

        // This callback is invoked on every audio frame. Don't do anything blocking here!
        auto onOutputFn = [&asciiboard](const microtone::AudioBuffer& outputData) {
            asciiboard.addOutputData(outputData);
        };

        // Create synthesizer
        auto synth = microtone::Synthesizer{weightedWaveTables, onOutputFn};

        // Listen to midi input
        auto midiInput = microtone::MidiInput();
        trySelectPort(midiInput);

        if (midiInput.isOpen()) {
            // Start midi input, update UI and synth when midi data changes
            midiInput.start([&synth, &asciiboard](int status, int note, int velocity) {
                synth.addMidiData(status, note, velocity);
                asciiboard.addMidiData(status, note, velocity);
            });
        }

        // Set up envelope controls (must be done after synth is started, for sample rate).
        initialControls.attack = 0.01;
        initialControls.decay = 0.1;
        initialControls.sustain = 0.8;
        initialControls.release = 0.01;
        auto envelope = microtone::Envelope(initialControls.attack,
                                            initialControls.decay,
                                            initialControls.sustain,
                                            initialControls.release,
                                            synth.sampleRate());

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

            auto shouldUpdateEnvelope = false;
            if (controls.attack != envelope.attack()) {
                envelope.setAttack(controls.attack);
                shouldUpdateEnvelope = true;
            }
            if (controls.decay != envelope.decay()) {
                envelope.setDecay(controls.decay);
                shouldUpdateEnvelope = true;
            }
            if (controls.sustain != envelope.sustain()) {
                envelope.setSustain(controls.sustain);
                shouldUpdateEnvelope = true;
            }
            if (controls.release != envelope.release()) {
                envelope.setRelease(controls.release);
                shouldUpdateEnvelope = true;
            }
            if (shouldUpdateEnvelope) {
                synth.setEnvelope(envelope);
            }
        };
        synth.start();

        // Blocks this thread
        asciiboard.loop(initialControls, onControlsChangedFn);

    } catch (microtone::MicrotoneException& e) {
        std::cout << fmt::format("Microtone error: {}", e.what()) << std::endl;
    }

    return 0;
}
