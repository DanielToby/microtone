#include <asciiboard.hpp>

#include <microtone/exception.hpp>
#include <microtone/midi_input.hpp>
#include <microtone/synthesizer.hpp>

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

        // Listen to midi input
        auto midiInput = microtone::MidiInput();
        selectPort(midiInput);

        // Create synthesizer
        auto synth = microtone::Synthesizer{[&asciiboard](const std::array<float, microtone::FRAMES_PER_BUFFER>& outputData) {
            asciiboard.addOutputData(outputData);
        }};

        // Start synthesier, update UI and synth when midi data changes
        midiInput.start([&synth, &asciiboard](int status, int note, int velocity) {
            synth.addMidiData(status, note, velocity);
            asciiboard.addMidiData(status, note, velocity);
        });

        // Callback invoked when asciiboard envelope is changed
        auto onEnvelopeChangedFn = [&synth](double attack, double decay, double sustain, double release) {
            synth.setEnvelope(microtone::Envelope{attack, decay, sustain, release, synth.sampleRate()});
        };

        asciiboard.loop(onEnvelopeChangedFn);

    } catch (microtone::MicrotoneException& e) {
        std::cout << fmt::format("Microtone error: {}", e.what()) << std::endl;
    }

    return 0;
}
