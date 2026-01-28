#pragma once

#include <ftxui/component/component.hpp>

#include "common/midi_handle.hpp"

#include <unordered_set>

namespace asciiboard {

namespace detail {

[[nodiscard]] inline bool isBlackKeyPixel(int x, int y) {
    if (y > 3) {
        return false;
    }
    // C#
    if (x == 2 || x == 3) {
        return true;
    }
    // D#
    if (x == 5 || x == 6) {
        return true;
    }
    // F#
    if (x == 11 || x == 12) {
        return true;
    }
    // G#
    if (x == 14 || x == 15) {
        return true;
    }
    // A#
    if (x == 17 || x == 18) {
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
            auto width = 20;
            auto height = 6;
            auto c = Canvas(width, height);
            for (auto x = 0; x <= width; ++x) {
                for (auto y = 0; y <= height; ++y) {
                    if (detail::isBlackKeyPixel(x, y)) {
                        c.DrawBlock(x, y, true, Color::Black);
                    } else {
                        c.DrawBlock(x, y, true, Color::White);
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
    [[nodiscard]] static bool isBlackKey(const int key) {
        const auto v = key % 12;
        return v == 1 || v == 3 || v == 6 || v == 8 || v == 10;
    }

    common::midi::Keyboard _keyboard;
};

}
