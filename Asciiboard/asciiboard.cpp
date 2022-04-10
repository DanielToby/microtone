#include <asciiboard.hpp>

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

#include <fmt/format.h>

#include <unordered_set>
#include <string>

namespace asciiboard {

using namespace ftxui;

class Asciiboard::impl {
public:
    impl() :
        _screen{ScreenInteractive::Fullscreen()},
        _lastOutputBuffer{},
        _activeMidiNotes{},
        _sustainedMidiNotes{},
        _sustainPedalOn{false} {}

    // No allocation can take place here (called every frame)
    void addOutputData(const microtone::AudioBuffer& data) {
        std::copy(std::begin(data), std::end(data), std::begin(_lastOutputBuffer));
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


    void loop(const OnEnvelopeChangedFn& onEnvelopeChangedFn) {
        auto attack = std::string{"0.01"};
        auto decay = std::string{"0.1"};
        auto sustain = std::string{"0.8"};
        auto release = std::string{"0.01"};

        Component attackInput = Input(&attack, "Attack");
        Component decayInput = Input(&decay, "Decay");
        Component sustainInput = Input(&sustain, "Sustain");
        Component releaseInput = Input(&release, "Decay");

        auto submitEnvelope = [&onEnvelopeChangedFn, &attack, &decay, &sustain, &release]() {
            try {
                auto attackDouble = std::stod(attack);
                auto decayDouble = std::stod(decay);
                auto sustainDouble = std::stod(sustain);
                auto releaseDouble = std::stod(release);
                onEnvelopeChangedFn(attackDouble, decayDouble, sustainDouble, releaseDouble);
            } catch (...) {

            }
        };

        auto envelopeControls = Container::Vertical({
            attackInput,
            decayInput,
            sustainInput,
            releaseInput,
            Button("Submit", submitEnvelope),
        });

        auto scaleFactor = 0.5;
        auto oscilloscopeHeight = 60;
        auto oscilloscope = Renderer([&] {
            auto width = 200;
            auto c = Canvas(width, oscilloscopeHeight);
            for (auto i = 0; i < width - 1; ++i) {
                if (i < static_cast<int>(_lastOutputBuffer.size())) {
                    c.DrawPointLine(i,
                                    (_lastOutputBuffer[i] * oscilloscopeHeight * scaleFactor) + (oscilloscopeHeight / 2),
                                    i + 1,
                                    (_lastOutputBuffer[i + 1] * oscilloscopeHeight * scaleFactor) + (oscilloscopeHeight / 2));
                } else {
                    c.DrawPointLine(i, oscilloscopeHeight / 2, i + 1, oscilloscopeHeight / 2);
                }
            }

            return canvas(std::move(c));
        });

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

        auto synthesizer = Renderer([&oscilloscopeHeight, &scaleFactor, &oscilloscope, &keyboard, &activeNotes] {
            return vbox({
                vbox({
                    vbox({
                        text(fmt::format("Frequency [Mhz] x ({})", scaleFactor)) | hcenter,
                        oscilloscope->Render() | size(HEIGHT, LESS_THAN, oscilloscopeHeight) | hcenter
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
    microtone::AudioBuffer _lastOutputBuffer;
    std::unordered_set<int> _activeMidiNotes;
    std::unordered_set<int> _sustainedMidiNotes;
    bool _sustainPedalOn;
};

Asciiboard::Asciiboard() :
    _impl{new impl{}} {
}

Asciiboard::Asciiboard(Asciiboard&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Asciiboard& Asciiboard::operator=(Asciiboard&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

void Asciiboard::addOutputData(const microtone::AudioBuffer& data) {
    _impl->addOutputData(data);
}

void Asciiboard::addMidiData(int status, int note, int velocity) {
    _impl->addMidiData(status, note, velocity);
}

void Asciiboard::loop(const OnEnvelopeChangedFn& onEnvelopeChangedFn) {
    _impl->loop(onEnvelopeChangedFn);
}

Asciiboard::~Asciiboard() = default;

}
