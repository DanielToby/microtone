#pragma once

#include <array>
#include <ftxui/component/component.hpp>

#include "common/midi_handle.hpp"

namespace asciiboard {

namespace detail {

//! Piano notes are drawn in a buffer of size 21 for correct spacing.
constexpr std::size_t bufferSize = 21;

struct Interval {
    std::size_t low;
    std::size_t high;
};

const auto midiNoteRange = Interval{
    12, // C-1
    84  // C5
};

enum class KeyColor {
    Black, White
};

//! Properties that may be shared between keys.
struct KeyProperties {
    KeyColor color;
    std::size_t width;
    std::size_t height;
    ftxui::Color drawColor;
    ftxui::Color pressedColor;
};

//! Properties that are unique for each key.
struct KeyInOctave {
    std::size_t bufferIndex; //< Start position between [0, bufferSize)
    KeyProperties keyProperties;
};

struct PianoKey {
    KeyInOctave keyInOctave;
    std::size_t octave;
};

const auto whiteKeyProperties = KeyProperties{
    .color = KeyColor::White,
    .width = 3,
    .height = 8,
    .drawColor = ftxui::Color::BlueLight,
    .pressedColor = ftxui::Color::Blue};

const auto blackKeyProperties = KeyProperties{
    .color = KeyColor::Black,
    .width = 2,
    .height = 6,
    .drawColor = ftxui::Color::DeepPink1,
    .pressedColor = ftxui::Color::Magenta};

constexpr auto numMidiNotes = 12;
const std::array<KeyInOctave, numMidiNotes> drawnNotes = {{
    {0,  whiteKeyProperties}, // C
    {2,  blackKeyProperties}, // C#
    {3,  whiteKeyProperties}, // D
    {5,  blackKeyProperties}, // D#
    {6,  whiteKeyProperties}, // E
    {9,  whiteKeyProperties}, // F
    {11, blackKeyProperties}, // F#
    {12, whiteKeyProperties}, // G
    {14, blackKeyProperties}, // G#
    {15, whiteKeyProperties}, // A
    {17, blackKeyProperties}, // A#
    {18, whiteKeyProperties}  // B
}};

[[nodiscard]] inline PianoKey getPianoKey(std::size_t midiNote) {
    const auto note = midiNote % numMidiNotes;
    const auto octave = midiNote / numMidiNotes;
    return {drawnNotes[note], octave};
}

inline void drawBlockScaled(
    ftxui::Canvas& canvas,
    std::size_t x,
    std::size_t y,
    std::size_t scale,
    ftxui::Color color) {
    for (std::size_t dx = 0; dx < scale; ++dx) {
        for (std::size_t dy = 0; dy < scale; ++dy) {
            canvas.DrawPoint(
                x * scale + dx,
                y * scale + dy,
                true,
                color);
        }
    }
}

inline void drawKey(ftxui::Canvas& canvas, const PianoKey& key, bool isPressed, std::size_t scale) {
    const auto octaveOffset = key.octave * bufferSize;
    const auto& properties = key.keyInOctave.keyProperties;
    for (std::size_t x = 0; x < properties.width; ++x) {
        const auto xOffset = key.keyInOctave.bufferIndex + octaveOffset;
        for (std::size_t y = 0; y < properties.height; ++y) {
            drawBlockScaled(canvas, x + xOffset, y, scale, isPressed ? properties.pressedColor : properties.drawColor);
        }
    }
}

//! Draws all piano keys that satisfy the condition.
inline void drawKeysIf(ftxui::Canvas& canvas,
                       const common::midi::Keyboard& keyboard,
                       const std::function<bool(const PianoKey&)>& condition,
                       std::size_t scale) {
    for (auto i = midiNoteRange.low; i < midiNoteRange.high; ++i) {
        const auto pianoKey = getPianoKey(i);
        if (condition(pianoKey)) {
            drawKey(canvas, pianoKey, keyboard.isNotePressed(i), scale);
        }
    }
}

[[nodiscard]] inline ftxui::Canvas draw(const common::midi::Keyboard& keyboard) {
    constexpr std::size_t scale = 2; //< FTXUI block drawing needs a minimum scale factor of 2 to look right.

    // The black keys are all contained by the white keys, so the canvas is defined by white key properties.
    const std::size_t width = common::midi::NumMidiNodes * whiteKeyProperties.width * scale;
    const std::size_t height = whiteKeyProperties.height * scale;

    const auto isWhiteKey = [](const PianoKey& key) { return key.keyInOctave.keyProperties.color == KeyColor::White; };
    const auto isBlackKey = [](const PianoKey& key) { return key.keyInOctave.keyProperties.color == KeyColor::Black; };

    ftxui::Canvas canvas(width, height);

    // The z-order here matters; black keys obstruct the white keys.
    drawKeysIf(canvas, keyboard, isWhiteKey, scale);
    drawKeysIf(canvas, keyboard, isBlackKey, scale);

    return canvas;
}

}// namespace detail

class PianoRoll {
public:
    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;
        return Renderer([this] {
            return hbox({filler(),
                         canvas(detail::draw(_keyboard)),
                         filler()}) |
                   borderRounded | color(Color::RedLight);
        });
    }

    void setMidiKeyboard(const common::midi::Keyboard& keyboard) {
        _keyboard = keyboard;
    }

private:
    common::midi::Keyboard _keyboard;
};

}// namespace asciiboard
