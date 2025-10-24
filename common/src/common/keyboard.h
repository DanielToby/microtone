#pragma once

#include <array>

namespace common {

//! I don't know if this belongs here. I just didn't want to introduce dependencies between io and synth just for this.

//! A note on the keyboard. Velocity can be interpreted as "initial velocity".
struct Note {
    int velocity = 0;           // 0 if note is inactive.
    bool sustained = false;     // velocity won't be 0 if this is true.
};

//! The state of the midi controller. Active notes have >0 velocity.
struct Keyboard {
    std::array<Note, 127> notes;
};

}