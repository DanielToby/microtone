#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace io {

//! Recognized values in the `status` portion of a Midi Message.
enum class MidiStatusMessage {
    NoteOn = 0b10010000,
    NoteOff = 0b10000000,
    ControlChange = 0b10110000
};

//! Recognized values in the `note` portion of a Midi Message.
enum class MidiNoteMessage {
    SustainPedal = 64
};

//! Recognized values in the `velocity` portion of a Midi Message.
enum class MidiVelocityMessage {
    SustainPedalOnThreshold = 64
};

struct MidiMessage {
    int status;
    int note;
    int velocity;
};

using OnMidiDataFn = std::function<void(MidiMessage message)>;

class MidiInput {
public:
    MidiInput();
    MidiInput(const MidiInput&) = delete;
    MidiInput& operator=(const MidiInput&) = delete;
    MidiInput(MidiInput&&) noexcept;
    MidiInput& operator=(MidiInput&&) noexcept;
    ~MidiInput();

    [[nodiscard]] int portCount() const;
    [[nodiscard]] std::string portName(int portNumber) const;
    [[nodiscard]] bool isOpen() const;
    void openPort(int portNumber);
    void start(const OnMidiDataFn& onReceivedDataFn);
    void stop();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
