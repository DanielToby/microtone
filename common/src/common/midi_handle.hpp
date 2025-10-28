#pragma once

#include <array>
#include <functional>

#include <common/atomic_snapshot.hpp>

//! This could be "device" or something to reduce conceptual overlap with lib/io, but whatever.
namespace common::midi {

//! A note on the keyboard. Velocity is 0 if the note is off.
struct Note {
    int velocity = 0;

    [[nodiscard]] bool operator==(const Note& other) const { return velocity == other.velocity; }

    [[nodiscard]] bool isOn() const {
        return velocity > 0;
    }

    [[nodiscard]] bool isOff() const {
        return !this->isOn();
    }

    void triggerOn(int v) {
        velocity = v;
    }

    void triggerOff() {
        velocity = 0;
    }
};

//! The state of the midi controller. Active notes have >0 velocity.
struct Keyboard {
    std::array<Note, 127> notes;

    [[nodiscard]] bool operator==(const Keyboard& other) const { return notes == other.notes; }
    [[nodiscard]] bool operator!=(const Keyboard& other) const { return !(*this == other); }
};

// Notes are turned off when noteOff is pressed, unless the sustain pedal is on.
// Notes that have not been unpressed remain active when the pedal is released.

//! A thread safe midi state handle.
class MidiHandle {
public:
    MidiHandle() = default;

    void noteOn(int note, int velocity) {
        _pressedNotes.notes[note].triggerOn(velocity);
        this->getAndMutateAndSet([&](Keyboard& keyboard) {
            keyboard.notes[note].triggerOn(velocity);
        });
    }

    void noteOff(int note) {
        _pressedNotes.notes[note].triggerOff();
        this->getAndMutateAndSet([&](Keyboard& keyboard) {
            if (!_sustainPedalOn) {
                keyboard.notes[note].triggerOff();
            }
        });
    }

    void sustainOn() {
        _sustainPedalOn = true;
    }

    void sustainOff() {
        _sustainPedalOn = false;
        this->getAndMutateAndSet([&](Keyboard& keyboard) {
            for (auto note = 0; note <keyboard.notes.size(); ++note) {
                if (!isPressed(note)) {
                    keyboard.notes[note].triggerOff();
                }
            }
        });
    }

    [[nodiscard]] Keyboard getKeyboardState() const {
        return _keyboard.read();
    }

private:
    void getAndMutateAndSet(const std::function<void(Keyboard&)>& mutateFn) {
        auto copy = _keyboard.read();
        mutateFn(copy);
        _keyboard.write(copy);
    }

    [[nodiscard]] bool isPressed(int note) const {
        return _pressedNotes.notes[note].isOn();
    }

    // This data is shared with the reader (getKeyboardState).
    AtomicSnapshot<Keyboard> _keyboard;

    // This data is only important to the writer.
    Keyboard _pressedNotes;
    bool _sustainPedalOn = false;
};


}