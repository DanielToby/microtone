#include <display.hpp>

#include "ftxui/component/component.hpp"         // for Checkbox, Renderer, Horizontal, Vertical, Input, Menu, Radiobox, ResizableSplitLeft, Tab
#include "ftxui/component/component_base.hpp"    // for ComponentBase
#include "ftxui/component/component_options.hpp" // for MenuOption, InputOption
#include "ftxui/component/event.hpp"             // for Event, Event::Custom
#include "ftxui/component/screen_interactive.hpp"// for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
#include "ftxui/dom/flexbox_config.hpp"          // for FlexboxConfig
#include "ftxui/screen/color.hpp"                // for Color, Color::BlueLight, Color::RedLight, Color::Black, Color::Blue, Color::Cyan, Color::CyanLight, Color::GrayDark, Color::GrayLight, Color::Green, Color::GreenLight, Color::Magenta, Color::MagentaLight, Color::Red, Color::White, Color::Yellow, Color::YellowLight, Color::Default, Color::Palette256, ftxui
#include "ftxui/screen/color_info.hpp"           // for ColorInfo
#include "ftxui/screen/terminal.hpp"             // for Size, Dimensions


#include <unordered_set>

namespace microtone::internal {

using namespace ftxui;

class Display::impl {
public:
    impl() :
        _screen{ScreenInteractive::Fullscreen()},
        _data{},
        _activeMidiNotes{},
        _sustainedMidiNotes{},
        _sustainPedalOn{false} {}

    void addOutputData(const std::vector<float> &data) {
        _data = data;
        _screen.PostEvent(Event::Custom);
    }

    void addMidiData(int status, int note, [[maybe_unused]] int velocity) {
        if (status == 0b10010000) {
            _activeMidiNotes.insert(note);
        } else if (status == 0b10000000) {
            if (_sustainPedalOn) {
                _sustainedMidiNotes.insert(note);
            } else {
                _activeMidiNotes.erase(note);
            }
        } else if (status == 0b10110000) {
            // Control Change
            if (note == 64) {
                _sustainPedalOn = velocity > 64;
                if (!_sustainPedalOn) {
                    for (const auto& id : _sustainedMidiNotes) {
                        _activeMidiNotes.erase(id);
                    }
                    _sustainedMidiNotes.clear();
                }
            }
        }
        _screen.PostEvent(Event::Custom);
    }


    void loop() {
        auto oscilloscope = [this](int width, int height) {
            std::vector<int> output(width);
            for (auto i = 0; i < width; ++i) {
                if (i < static_cast<int>(_data.size())) {
                    output[i] = abs(_data[i]) * height;
                }
            }
            return output;
        };

        auto keyboard = [](int width, int height) {
            std::vector<int> output(width, 0);
            auto blackKeys = std::unordered_set<int>{1, 3, 6, 8, 10};
            for (auto i = 0; i < width; ++i) {
                if (blackKeys.find((i) % 12) == blackKeys.end()) {
                    // White key
                    output[i] = height - 1;
                }
            }
            return output;
        };

        auto activeNotes = [this](int width, [[maybe_unused]] int height) {
            std::vector<int> output(width, -1);
            for (auto i = 0; i < width; ++i) {
                if (_activeMidiNotes.find(i) != _activeMidiNotes.end() ||
                        _sustainedMidiNotes.find(i) != _sustainedMidiNotes.end()) {
                    output[i] = 1;
                }
            }
            return output;
        };

        auto synthesizer = Renderer([&oscilloscope, &keyboard, &activeNotes] {
            return vbox({
                vbox({
                    vbox({
                        text("Frequency [Mhz]") | hcenter,
                        hbox({
                            vbox({
                                text("2400 "),
                                filler(),
                                text("1200 "),
                                filler(),
                                text("0 "),
                            }),
                            graph(std::ref(oscilloscope)) | size(HEIGHT, EQUAL, 20),
                        }) | flex,
                    }) | borderRounded,
                    vbox({
                        hbox({
                            filler(),
                            graph(std::ref(activeNotes))
                                        | size(WIDTH, EQUAL, 66)
                                        | size(HEIGHT, EQUAL, 2) | color(Color::GreenLight),
                            filler()
                        }),
                        hbox({
                            filler(),
                            graph(std::ref(keyboard))
                                        | size(WIDTH, EQUAL, 66)
                                        | size(HEIGHT, EQUAL, 2),
                            filler()
                        })
                    }) | borderRounded
                 })
            });
        });

        auto main_renderer = Renderer(synthesizer, [&synthesizer] {
            return vbox({
                text("microtone") | bold | hcenter,
                synthesizer->Render()
            });
        });

        _screen.Loop(main_renderer);
    }

    ftxui::ScreenInteractive _screen;
    std::vector<float> _data;
    std::unordered_set<int> _activeMidiNotes;
    std::unordered_set<int> _sustainedMidiNotes;
    bool _sustainPedalOn;
};

Display::Display() :
    _impl{new impl{}} {
}

Display::Display(Display&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Display& Display::operator=(Display&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

void Display::addOutputData(const std::vector<float> &data) {
    _impl->addOutputData(data);
}

void Display::addMidiData(int status, int note, int velocity) {
    _impl->addMidiData(status, note, velocity);
}

void Display::loop() {
    _impl->loop();
}

Display::~Display() = default;

}
