#pragma once

#include <array>

#include <common/dirty_flagged.hpp>
#include <common/mutex_protected.hpp>

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

constexpr std::size_t NumMidiNodes = 127;

//! The state of the midi controller.
struct Keyboard {
    friend class KeyboardFactory;

    std::array<Note, NumMidiNodes> audibleNotes;

    [[nodiscard]] bool operator==(const Keyboard& other) const {
        return audibleNotes == other.audibleNotes && pressedNotes == other.pressedNotes && sustainOn == other.sustainOn;
    }
    [[nodiscard]] bool operator!=(const Keyboard& other) const {
        return !(*this == other);
    }

    [[nodiscard]] bool isNotePressed(const std::size_t note) const {
        if (note > NumMidiNodes) {
            throw std::out_of_range("Not a valid midi note.");
        }
        return pressedNotes[note].isOn();
    }

private:
    std::array<Note, 128> pressedNotes;
    bool sustainOn = false;
};

class KeyboardFactory {
public:
    [[nodiscard]] static Keyboard copyWithNoteOn(const Keyboard& k, int note, int velocity) {
        auto result = k;
        result.pressedNotes[note].triggerOn(velocity);
        result.audibleNotes[note].triggerOn(velocity);
        return result;
    }

    [[nodiscard]] static Keyboard copyWithNoteOff(const Keyboard& k, int note) {
        auto result = k;
        result.pressedNotes[note].triggerOff();
        if (!result.sustainOn) {
            result.audibleNotes[note].triggerOff();
        }
        return result;
    }

    [[nodiscard]] static Keyboard copyWithSustainOn(const Keyboard& k) {
        auto result = k;
        result.sustainOn = true;
        return result;
    }

    [[nodiscard]] static Keyboard copyWithSustainOff(const Keyboard& k) {
        auto result = k;
        for (auto note = 0; note < result.audibleNotes.size(); ++note) {
            if (!result.pressedNotes[note].isOn()) {
                result.audibleNotes[note].triggerOff();
            }
        }
        result.sustainOn = false;
        return result;
    }
};

//! A thread safe midi state handle. NumReaders must be known to the caller.
template <std::size_t NumReaders>
class MidiHandle {
public:
    MidiHandle() = default;

    void noteOn(int note, int velocity) {
        _keyboard.write(KeyboardFactory::copyWithNoteOn(_keyboard.quietRead(), note, velocity));
    }

    void noteOff(int note) {
        _keyboard.write(KeyboardFactory::copyWithNoteOff(_keyboard.quietRead(), note));
    }

    void sustainOn() {
        _keyboard.write(KeyboardFactory::copyWithSustainOn(_keyboard.quietRead()));
    }

    void sustainOff() {
        _keyboard.write(KeyboardFactory::copyWithSustainOff(_keyboard.quietRead()));
    }

    [[nodiscard]] std::size_t registerReader() const {
        return _keyboard.registerReader();
    }

    [[nodiscard]] Keyboard read(std::size_t readerId) const {
        return _keyboard.read(readerId);
    }

    [[nodiscard]] bool hasChanges(std::size_t readerId) const {
        return _keyboard.isDirty(readerId);
    }

private:
    DirtyFlagged<MutexProtected<Keyboard>, NumReaders> _keyboard;
};

//! For convenience while I plumb NumMidiReaders into more contexts.
using TwoReaderMidiHandle = MidiHandle<2>;

}