#pragma once

#include <microtone/microtone_platform.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace microtone {

enum class MidiStatusMessage {
    NoteOn = 0b10010000,
    NoteOff = 0b10000000,
    ControlChange = 0b10110000
};

using OnMidiDataFn = std::function<void(int status, int note, int velocity)>;

class MidiInput {
public:
    MidiInput();
    MidiInput(const MidiInput&) = delete;
    MidiInput& operator=(const MidiInput&) = delete;
    MidiInput(MidiInput&&) noexcept;
    MidiInput& operator=(MidiInput&&) noexcept;
    ~MidiInput();

    int portCount() const;
    std::string portName(int portNumber) const;
    void openPort(int portNumber);
    void start(OnMidiDataFn onReceivedDataFn);
    void stop();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
