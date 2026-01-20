#pragma once

#include <ftxui/component/component.hpp>

#include "common/midi_handle.hpp"

#include <unordered_set>

namespace asciiboard {

class PianoRoll {
public:
    PianoRoll() {
        using namespace ftxui;

        _basePianoRoll = [](int width, int height) {
            std::vector<int> output(width, 0);
            for (auto i = 0; i < width; ++i) {
                if (!isBlackKey(i)) {
                    // White key
                    output[i] = height - 1;
                }
            }
            return output;
        };

        _activeNotes = [&](int width, [[maybe_unused]] int height) {
            std::vector<int> output(width, -1);
            for (auto i = 0; i < std::min(width, static_cast<int>(_keyboard.audibleNotes.size())); ++i) {
                if (_keyboard.audibleNotes[i].isOn()) {
                    output[i] = 1;
                }
            }
            return output;
        };

        _component = Renderer([this]() {
            return hbox({filler(),
                         vbox({graph(std::ref(_activeNotes)) | size(HEIGHT, EQUAL, 2) | color(Color::GreenLight),
                               graph(std::ref(_basePianoRoll)) | size(HEIGHT, EQUAL, 2) | color(Color::Default)}) |
                             size(WIDTH, EQUAL, 66),
                         filler()}) |
                   borderRounded | color(Color::RedLight);
        });
    }

    [[nodiscard]] const ftxui::Component& component() const {
        return _component;
    }

    void setMidiKeyboard(const common::midi::Keyboard& keyboard) {
        _keyboard = keyboard;
    }

private:
    [[nodiscard]] static bool isBlackKey(const int key) {
        const auto v = key % 12;
        return v == 1 || v == 3 || v == 6 || v == 8 || v == 10;
    }

    ftxui::Component _component;
    common::midi::Keyboard _keyboard;

    std::function<std::vector<int>(int, int)> _basePianoRoll;
    std::function<std::vector<int>(int, int)> _activeNotes;
};

}