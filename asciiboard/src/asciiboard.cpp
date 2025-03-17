#include <asciiboard/asciiboard.hpp>

#include "ftxui/component/component.hpp"         // for Checkbox, Renderer, Horizontal, Vertical, Input, Menu, Radiobox, ResizableSplitLeft, Tab
#include "ftxui/component/component_base.hpp"    // for ComponentBase
#include "ftxui/component/component_options.hpp" // for MenuOption, InputOption
#include "ftxui/component/event.hpp"             // for Event, Event::Custom
#include "ftxui/component/screen_interactive.hpp"// for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
#include "ftxui/screen/color.hpp"                // for Color, Color::BlueLight, Color::RedLight, Color::Black, Color::Blue, Color::Cyan, Color::CyanLight, Color::GrayDark, Color::GrayLight, Color::Green, Color::GreenLight, Color::Magenta, Color::MagentaLight, Color::Red, Color::White, Color::Yellow, Color::YellowLight, Color::Default, Color::Palette256, ftxui

#include <fmt/format.h>

#include <microtone/midi_input.hpp>

#include <string>
#include <unordered_set>

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
        // auto midiStatus = static_cast<microtone::MidiStatusMessage>(status);

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

    void loop(const SynthControls& initialControls, const OnControlsChangedFn& onControlsChangedFn) {
        auto controls = SynthControls{initialControls};

        // Instructions
        auto showInstructions = true;
        auto closeInstructions = Button("Ok", [&showInstructions] { showInstructions = false; });
        auto instructions = Renderer(closeInstructions, [&closeInstructions] {
            return vbox({text("Press <enter> to submit changes to the controls. Press 'q' to quit."),
                         hbox({filler(),
                               closeInstructions->Render(),
                               filler()})}) |
                   hcenter | border;
        });

        // =========== Oscillators tab ===========

        // Oscillator Controls
        auto sine = static_cast<int>(controls.sineWeight * 100);
        auto square = static_cast<int>(controls.squareWeight * 100);
        auto triangle = static_cast<int>(controls.triangleWeight * 100);
        auto sineSlider = Slider("Sine:", &sine, 0, 100, 1);
        auto squareSlider = Slider(" Square:", &square, 0, 100, 1);
        auto triangleSlider = Slider(" Triangle:", &triangle, 0, 100, 1);

        auto oscillatorControlsContainer = Container::Horizontal({
            sineSlider,
            squareSlider,
            triangleSlider,
        });

        auto oscillatorControls = Renderer(oscillatorControlsContainer,
                                           [&sineSlider, &squareSlider, &triangleSlider]() {
                                               return hbox({sineSlider->Render(),
                                                            squareSlider->Render(),
                                                            triangleSlider->Render()});
                                           });

        // Oscilloscope
        auto scaleFactor = 0.5;
        auto graphHeight = 40;
        auto oscilloscope = Renderer([&] {
            auto width = static_cast<int>(_lastOutputBuffer.size());
            auto c = Canvas(width, graphHeight);
            for (auto i = 0; i < width - 1; ++i) {
                c.DrawPointLine(i,
                                (_lastOutputBuffer[i] * graphHeight * scaleFactor) + (graphHeight / 2),
                                i + 1,
                                (_lastOutputBuffer[i + 1] * graphHeight * scaleFactor) + (graphHeight / 2),
                                Color::Purple);
            }

            return vbox({
                text(fmt::format("Frequency [Mhz] x ({})", scaleFactor)) | hcenter,
                canvas(std::move(c)) | hcenter
            });
        });

        auto oscillatorsContainer = Container::Vertical({
                                                  oscillatorControls,
                                                  oscilloscope});

        auto oscillators = Renderer(oscillatorsContainer, [&] {
            return vbox({
                oscilloscope->Render() | flex,
                oscillatorControls->Render(),
            }) | borderRounded | hcenter | color(Color::BlueLight);
        });

        // =========== Envelope tab ===========

        // Envelope Controls
        auto attack = static_cast<int>(controls.attack * 100);
        auto decay = static_cast<int>(controls.decay * 100);
        auto sustain = static_cast<int>(controls.sustain * 100);
        auto release = static_cast<int>(controls.release * 100);
        auto attackSlider = Slider("Attack:", &attack, 1, 100, 1);
        auto decaySlider = Slider(" Decay:", &decay, 1, 100, 1);
        auto sustainSlider = Slider(" Sustain:", &sustain, 0, 100, 1);
        auto releaseSlider = Slider(" Release:", &release, 1, 100, 1);

        auto envelopeControlsContainer = Container::Horizontal({
            attackSlider,
            decaySlider,
            sustainSlider,
            releaseSlider
        });

        auto envelopeControls = Renderer(envelopeControlsContainer,
                                         [&attackSlider, &decaySlider, &sustainSlider, &releaseSlider] () {
                                             return hbox({
                                                 attackSlider->Render(),
                                                 decaySlider->Render(),
                                                 sustainSlider->Render(),
                                                 releaseSlider->Render()
                                             });
                                         });

        auto envelopeGraph = Renderer([&] {
            auto width = 80;
            auto c = Canvas(width, graphHeight);
            auto effectiveHeight = graphHeight - 5;
             /*
             *           / \
             *         /    \
             *       /       \___________
             *     /                     \
             *     0     x1  x2          x3
             *     a     d   s           r
             */

            // The "sustain" phase always has a width of w/4.
            auto sustainWidth = width / 4;

            // The remaining width is split among the other phases.
            auto remainingWidth = (width - sustainWidth);

            auto totalTime = attack + decay + release;
            auto x1 = (static_cast<double>(attack) / totalTime) * remainingWidth;
            auto x2 = ((static_cast<double>(attack) + decay) / totalTime) * remainingWidth;
            auto x3 = x2 + sustainWidth;

            auto sustainHeight = effectiveHeight - (effectiveHeight * (static_cast<double>(sustain) / 100));

            c.DrawPointLine(0, effectiveHeight, x1, 0, Color::Cyan);
            c.DrawPointLine(x1, 0, x2, sustainHeight, Color::BlueLight);
            c.DrawPointLine(x2, sustainHeight, x3, sustainHeight, Color::Purple);
            c.DrawPointLine(x3, sustainHeight, width, effectiveHeight, Color::Red);

            return vbox({
                text(fmt::format("Envelope (ms)", scaleFactor)) | hcenter,
                canvas(std::move(c)) | hcenter
            });
        });

        auto envelopeContainer = Container::Vertical({ envelopeControls });

        auto envelope = Renderer(envelopeContainer, [&] {
            return vbox({
                         hbox({
                           filler(),
                           envelopeGraph->Render(),
                           filler()
                       }),
                envelopeControls->Render()
            }) | borderRounded | color(Color::BlueLight);
        });

        auto tab_index = 0;
        auto tab_entries = std::vector<std::string>{ "oscillators", "envelope" };
        auto selectedTab = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
        auto tabContent = Container::Tab({ oscillators, envelope }, &tab_index);

        // Piano roll
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

        auto pianoRoll = Renderer([&activeNotes, &keyboard]() {
            return hbox({
                        filler(),
                        vbox({
                            graph(std::ref(activeNotes)) | size(HEIGHT, EQUAL, 2) | color(Color::GreenLight),
                           graph(std::ref(keyboard)) | size(HEIGHT, EQUAL, 2) | color(Color::Default)
                        }) | size(WIDTH, EQUAL, 66),
                        filler()
                   }) | borderRounded | color(Color::RedLight);
        });

        auto mainContents = Container::Vertical({
            instructions,
            selectedTab,
            tabContent,
            pianoRoll
        });

        auto mainRenderer = Renderer(mainContents, [&] {
            Element document = vbox({
                text("microtone") | bold | hcenter,
                selectedTab->Render(),
                tabContent->Render(),
                pianoRoll->Render()
            });

            if (showInstructions) {
            document = dbox({
                             document,
                             instructions->Render() | clear_under | center,
                             });
            }
            return document;
        });

        auto eventListener = CatchEvent(mainRenderer, [&](Event event) {
            if (event == Event::Character('q')) {
                _screen.ExitLoopClosure()();
                return true;
            } else if (event == Event::Return) {
                controls.sineWeight = static_cast<double>(sine) / 100;
                controls.squareWeight = static_cast<double>(square) / 100;
                controls.triangleWeight = static_cast<double>(triangle) / 100;

                controls.attack = static_cast<double>(attack) / 100;
                controls.decay = static_cast<double>(decay) / 100;
                controls.sustain = static_cast<double>(sustain) / 100;
                controls.release = static_cast<double>(release) / 100;

                onControlsChangedFn(controls);
            }
            return false;
        });

        _screen.Loop(eventListener);
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

void Asciiboard::loop(const SynthControls& initialControls, const OnControlsChangedFn& onControlsChangedFn) {
    _impl->loop(initialControls, onControlsChangedFn);
}

Asciiboard::~Asciiboard() = default;

}
