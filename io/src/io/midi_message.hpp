#pragma once

#include <array>
#include <atomic>

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

}