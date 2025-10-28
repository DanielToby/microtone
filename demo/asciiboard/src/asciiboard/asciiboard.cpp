#include <asciiboard/asciiboard.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include <fmt/format.h>

#include <io/midi_input_stream.hpp>

#include <string>
#include <unordered_set>

namespace asciiboard {

using namespace ftxui;

class Asciiboard::impl {
public:
    explicit impl() :
        _latestAudioBlock{},
        _screen{ScreenInteractive::Fullscreen()} {}

    void addOutputData(const common::audio::FrameBlock& audioBlock) {
        if (_latestAudioBlock != audioBlock) {
            _latestAudioBlock = audioBlock;
            _screen.PostEvent(Event::Custom);
        }
    }

    void updateMidiKeyboard(const common::midi::Keyboard& keyboard) {
        if (_keyboard != keyboard) {
            _keyboard = keyboard;
            _screen.PostEvent(Event::Custom);
        }
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
            auto width = static_cast<int>(_latestAudioBlock.size());
            auto c = Canvas(width, graphHeight);
            for (auto i = 0; i < width - 1; ++i) {
                c.DrawPointLine(i,
                                (_latestAudioBlock[i] * graphHeight * scaleFactor) + (graphHeight / 2),
                                i + 1,
                                (_latestAudioBlock[i + 1] * graphHeight * scaleFactor) + (graphHeight / 2),
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
        auto attack = static_cast<int>(controls.adsr.attack * 100);
        auto decay = static_cast<int>(controls.adsr.decay * 100);
        auto sustain = static_cast<int>(controls.adsr.sustain * 100);
        auto release = static_cast<int>(controls.adsr.release * 100);
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
                if (_keyboard.notes[i].isOn()) {
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
                controls.sineWeight = sine / 100.;
                controls.squareWeight = square / 100.;
                controls.triangleWeight = triangle / 100.;
                controls.adsr = {attack / 100., decay / 100., sustain / 100., release / 100.};

                onControlsChangedFn(controls);
            }
            return false;
        });

        _screen.Loop(eventListener);
    }

    ftxui::ScreenInteractive _screen;
    common::audio::FrameBlock _latestAudioBlock;
    common::midi::Keyboard _keyboard;
};

Asciiboard::Asciiboard() : _impl{std::make_unique<impl>()} {};

Asciiboard::Asciiboard(Asciiboard&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Asciiboard& Asciiboard::operator=(Asciiboard&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

void Asciiboard::addOutputData(const common::audio::FrameBlock& lastAudioBlock) {
    _impl->addOutputData(lastAudioBlock);
}

void Asciiboard::updateMidiKeyboard(const common::midi::Keyboard& keyboard) {
    _impl->updateMidiKeyboard(keyboard);
}

void Asciiboard::loop(const SynthControls& initialControls, const OnControlsChangedFn& onControlsChangedFn) {
    _impl->loop(initialControls, onControlsChangedFn);
}

Asciiboard::~Asciiboard() = default;

}
