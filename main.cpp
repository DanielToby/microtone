#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/midi_input.hpp>
#include <microtone/synthesizer.hpp>

#include <fmt/format.h>

#include <iostream>
#include <string>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    microtone::Log::init();

    try {
        auto midiInput = microtone::MidiInput();

        auto selectedPort = -1;
        auto numPorts = midiInput.portCount();
        if (numPorts == 0) {
            std::cout << "No ports are available. Exiting." << std::endl;
            return 0;
        } else if (numPorts == 1) {
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

                if (selectedPort < 0 || selectedPort > numPorts - 1) {
                    std::cout << "Port number out of range." << std::endl;
                }
                std::cout << std::endl;
            }
        }

        midiInput.openPort(selectedPort);

        auto synth = microtone::Synthesizer{};

        auto midiCallback = [&synth](double timeStamp, const std::string& message) {
            auto numBytes = message.size();
            for (unsigned int i = 0; i < numBytes; ++i) {
                std::cout << fmt::format("Byte {} = '{}'.", i, static_cast<int>(message[i])) << std::endl;
            }
            if (numBytes > 0) {
                std::cout << fmt::format("Timestamp = {}.", timeStamp) << std::endl;
            }

            if (message.size() == 3) {
                auto unknown = static_cast<int>(message[0]);
                auto midiNote = static_cast<int>(message[1]);
                auto velocity = static_cast<int>(message[2]);

                synth.addNoteData(midiNote, velocity, timeStamp);
            }
        };
        midiInput.start(midiCallback);

        std::cout << "\nReading MIDI input... press <enter> to quit.\n";
        auto input = char{};
        std::cin.get(input);

    } catch (microtone::MicrotoneException& e) {
        std::cout << fmt::format("Microtone error: {}", e.what()) << std::endl;
    }

    return 0;
}
