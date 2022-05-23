#include <asciiboard.hpp>

#include <microtone/exception.hpp>
#include <microtone/midi_input.hpp>
#include <microtone/synthesizer/audio_buffer.hpp>
#include <microtone/synthesizer/synthesizer.hpp>
#include <microtone/synthesizer/weighted_wavetable.hpp>

#include <fmt/format.h>

#include <iostream>
#include <string>

void selectPort(microtone::MidiInput& midiInput) {
    auto selectedPort = -1;
    auto numPorts = midiInput.portCount();
    while (numPorts == 0) {
        std::cout << "No midi ports are available. Press <enter> to retry." << std::endl;
        std::cin.get();
    }
    if (numPorts == 1) {
        selectedPort = 0;
    } else {
        while (selectedPort < 0 || selectedPort > numPorts - 1) {
            std::cout << fmt::format("Please choose a port between 1 and {}.", numPorts) << std::endl;
            for (auto i = 0; i < numPorts; ++i) {
                std::cout << fmt::format("{0}. {1}", i + 1, midiInput.portName(i)) << std::endl;
            }

            auto input = std::string{};
            std::getline(std::cin, input);
            try {
                selectedPort = std::stoi(input) - 1;
            } catch (...) {
                std::cout << "Enter a number." << std::endl;
            }
        }
    }
    midiInput.openPort(selectedPort);
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
        auto sineWave = microtone::WaveTable{};
        for (auto i = 0; i < microtone::WAVETABLE_LENGTH; ++i) {
            sineWave[i] = std::sin(2.0 * M_PI * i / microtone::WAVETABLE_LENGTH);
        }

        auto squareWave = microtone::WaveTable{};
        for (auto i = 0; i < microtone::WAVETABLE_LENGTH; ++i) {
            squareWave[i] = std::sin(2.0 * M_PI * i / microtone::WAVETABLE_LENGTH) > 0 ? 1.0 : -1.0;
        }

        auto triangleWave = microtone::WaveTable{};
        for (auto i = 0; i < microtone::WAVETABLE_LENGTH; ++i) {
            triangleWave[i] = std::asin(std::sin(2.0 * M_PI * i / microtone::WAVETABLE_LENGTH)) * (2.0 / M_PI);
        }

        weightedWaveTables.emplace_back(sineWave, initialControls.sineWeight);
        weightedWaveTables.emplace_back(squareWave, initialControls.squareWeight);
        weightedWaveTables.emplace_back(triangleWave, initialControls.triangleWeight);

        // This callback is invoked on every audio frame. Don't do anything blocking here!
        auto onOutputFn = [&asciiboard](const microtone::AudioBuffer& outputData) {
            asciiboard.addOutputData(outputData);
        };

        // Create synthesizer
        auto synth = microtone::Synthesizer{weightedWaveTables, onOutputFn};

        // Listen to midi input
        auto midiInput = microtone::MidiInput();
        selectPort(midiInput);

        // Start midi input, update UI and synth when midi data changes
        midiInput.start([&synth, &asciiboard](int status, int note, int velocity) {
            synth.addMidiData(status, note, velocity);
            asciiboard.addMidiData(status, note, velocity);
        });

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
