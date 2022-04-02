#include <display.hpp>

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

//        // Display
//        auto display = microtone::internal::Display();
//        display.loop();

        // Listen to midi input
        auto midiInput = microtone::MidiInput();
        selectPort(midiInput);

        // Start synthesizer
        auto synth = microtone::Synthesizer{};
        auto midiCallback = [&synth](int status, int note, int velocity) {
            synth.addNoteData(status, note, velocity);
        };
        midiInput.start(midiCallback);

        std::cin.get();

    } catch (microtone::MicrotoneException& e) {
        std::cout << fmt::format("Microtone error: {}", e.what()) << std::endl;
    }

    return 0;
}
