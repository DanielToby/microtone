#pragma once

#include <ftxui/component/component.hpp>

#include "common/midi_handle.hpp"

#include <unordered_set>

namespace asciiboard {

namespace detail {

[[nodiscard]] inline bool isBlackKeyPixel(int x, int y) {
    if (y > 8) {
        return false;
    }
    // C#
    if (x >= 4 && x <= 6) {
        return true;
    }
    // D#
    if (x >= 10 && x <= 12) {
        return true;
    }
    // F#
    if (x >= 22 && x <= 24) {
        return true;
    }
    // G#
    if (x >= 28 && x <= 30) {
        return true;
    }
    // A#
    if (x >= 34 && x <= 36) {
        return true;
    }
    return false;
}

}

class PianoRoll {
public:
    PianoRoll() = default;

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;
        return Renderer([this] {
            auto width = 42;
            auto height = 14;
            auto c = Canvas(width, height);
            for (auto x = 0; x < width; ++x) {
                for (auto y = 0; y < height; ++y) {
                    if (detail::isBlackKeyPixel(x, y)) {
                        c.DrawBlock(x, y, true, Color::DarkBlue);
                    } else {
                        c.DrawBlock(x, y, true, Color::DarkViolet);
                    }
                }
            }

            return hbox({filler(),
                         canvas(c),
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

}
