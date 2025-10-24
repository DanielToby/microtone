#pragma once

#include <array>
#include <functional>

#include <common/atomic_snapshot.hpp>

//! This could be "device" or something to reduce conceptual overlap with lib/io, but whatever.
namespace common::midi {

//! A note on the keyboard. Velocity can be interpreted as "initial velocity".
struct Note {
    int velocity = 0;           // 0 if note is inactive.
    bool sustained = false;     // velocity won't be 0 if this is true.
};

//! The state of the midi controller. Active notes have >0 velocity.
struct Keyboard {
    std::array<Note, 127> notes;
};

//! A thread safe midi state handle.
class MidiHandle {
public:
    MidiHandle() = default;

    void noteOn(int note, int velocity) {
        this->write([&](Keyboard& keyboard) {
            keyboard.notes[note].velocity = velocity;
            keyboard.notes[note].sustained = false;
        });
    }

    void noteOff(int note) {
        this->write([&](Keyboard& keyboard) {
            if (_sustainPedalOn) {
                keyboard.notes[note].sustained = true;
            } else {
                keyboard.notes[note].velocity = 0;
                keyboard.notes[note].sustained = false;
            }
        });
    }

    void sustainOn() {
        _sustainPedalOn = true;
    }

    void sustainOff() {
        _sustainPedalOn = false;
        this->write([&](Keyboard& keyboard) {
            for (auto& [velocity, sustained] : keyboard.notes) {
                if (sustained) {
                    velocity = 0;
                    sustained = false;
                }
            }
        });
    }

    [[nodiscard]] Keyboard getKeyboardState() const {
        return _keyboard.read();
    }

private:
    void write(const std::function<void(Keyboard&)>& mutateFn) {
        auto copy = _keyboard.read();
        mutateFn(copy);
        _keyboard.write(copy);
    }

    AtomicSnapshot<Keyboard> _keyboard;
    bool _sustainPedalOn = false; // Only touched when writing.
};


}