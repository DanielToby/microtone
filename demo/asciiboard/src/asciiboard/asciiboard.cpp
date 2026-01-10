#include <asciiboard/asciiboard.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <fmt/format.h>

#include "common/log.hpp"
#include "io/midi_input_stream.hpp"

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

    void loop(const SynthControls& initialControls, const OnControlsChangedFn& onControlsChangedFn, const OnAboutToQuitFn& onAboutToQuit) {
        // Modified by the UI.
        auto controls = SynthControls{initialControls};

        // Constants.
        constexpr auto graphHeight = 120;
        constexpr auto scaleFactor = 0.5;
        constexpr auto envelopeGraphWidth = 80;


        // =========== Info ===========
        auto showInfo = true;
        auto closeInfo = Button("Ok", [&showInfo] { showInfo = false; });
        auto instructionMessage = "Press <enter> to submit changes to the controls. Press 'q' to quit.";
        auto logLocationMessage = fmt::format("Logs are written to: {}", common::Log::getDefaultLogfilePath());
        auto info = Renderer(closeInfo, [&closeInfo, &instructionMessage, &logLocationMessage] {
            return vbox({text(instructionMessage) | hcenter,
                         separatorEmpty() | size(HEIGHT, EQUAL, 1),
                         text(logLocationMessage) | hcenter,
                         separatorEmpty() | size(HEIGHT, EQUAL, 1),
                         hbox({filler(),
                               closeInfo->Render(),
                               filler()})}) |
                   hcenter | border;
        });

        // =========== Oscillators tab ===========
        auto oscilloscope = Renderer([&] {
            auto width = static_cast<int>(_latestAudioBlock.size());
            auto c = Canvas(width, graphHeight);
            for (auto i = 0; i < width - 1; ++i) {
                c.DrawPointLine(i,
                                static_cast<int>(_latestAudioBlock[i] * graphHeight * scaleFactor) + (graphHeight / 2),
                                i + 1,
                                static_cast<int>(_latestAudioBlock[i + 1] * graphHeight * scaleFactor) + (graphHeight / 2),
                                Color::Purple);
            }

            return vbox({
                text(fmt::format("Frequency [Mhz] x ({})", scaleFactor)) | hcenter,
                vbox({
                    filler(),
                    canvas(std::move(c)) | hcenter,
                    filler(),
                }) | flex,
            });
        });

        auto sineSlider = Slider("Sine:", &controls.sineWeight_pct, 0, 100, 1);
        auto squareSlider = Slider(" Square:", &controls.squareWeight_pct, 0, 100, 1);
        auto triangleSlider = Slider(" Triangle:", &controls.triangleWeight_pct, 0, 100, 1);

        auto oscillatorControlsContainer = Container::Horizontal({
            sineSlider,
            squareSlider,
            triangleSlider,
        });

        auto oscillatorMixer = Renderer(oscillatorControlsContainer,
                                        [&sineSlider, &squareSlider, &triangleSlider]() {
                                            return hbox({sineSlider->Render(),
                                                         squareSlider->Render(),
                                                         triangleSlider->Render()});
                                        });

        // Gain and LFO Frequency
        auto gainSlider = Slider("Gain:", &controls.gain, .1, 1., .1);
        auto lfoFrequencyHzSlider = Slider("LFO Frequency (Hz):", &controls.lfoFrequency_Hz, .01, 20, .1);
        auto lfoGainSlider = Slider("LFO Gain:", &controls.lfoGain, 0., 1., .1);
        auto gainAndLfoFrequencyContainer = Container::Horizontal({gainSlider, lfoFrequencyHzSlider, lfoGainSlider});
        auto gainAndLfoFrequencyControls = Renderer(gainAndLfoFrequencyContainer,
                                                    [&gainSlider, &lfoFrequencyHzSlider, &lfoGainSlider]() {
                                                        return hbox({gainSlider->Render(),
                                                                     lfoFrequencyHzSlider->Render(),
                                                                     lfoGainSlider->Render()});
                                                    });

        auto oscillatorControls = Container::Vertical({oscillatorMixer, gainAndLfoFrequencyControls});
        auto oscillatorsContainer = Container::Vertical({oscilloscope, oscillatorControls});
        auto oscillatorsTab = Renderer(oscillatorsContainer, [&] {
            return vbox({
                       oscilloscope->Render() | flex,
                       oscillatorControls->Render(),
                   }) |
                   borderRounded | hcenter | color(Color::BlueLight);
        });

        // =========== Envelope tab ===========
        auto envelopeGraph = Renderer([&] {
            auto c = Canvas(envelopeGraphWidth, graphHeight);
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
            auto sustainWidth = envelopeGraphWidth / 4;

            // The remaining width is split among the other phases.
            auto remainingWidth = (envelopeGraphWidth - sustainWidth);

            auto totalTime = controls.attack_pct + controls.decay_pct + controls.release_pct;
            auto x1 = (static_cast<double>(controls.attack_pct) / totalTime) * remainingWidth;
            auto x2 = ((static_cast<double>(controls.attack_pct) + controls.decay_pct) / totalTime) * remainingWidth;
            auto x3 = x2 + sustainWidth;

            auto sustainHeight = effectiveHeight - (effectiveHeight * (static_cast<double>(controls.sustain_pct) / 100));

            c.DrawPointLine(0, effectiveHeight, x1, 0, Color::Cyan);
            c.DrawPointLine(x1, 0, x2, sustainHeight, Color::BlueLight);
            c.DrawPointLine(x2, sustainHeight, x3, sustainHeight, Color::Purple);
            c.DrawPointLine(x3, sustainHeight, envelopeGraphWidth, effectiveHeight, Color::Red);

            return vbox({text("Envelope (ms)") | hcenter, canvas(std::move(c)) | hcenter});
        });
        auto envelopeContainer = Renderer(envelopeGraph,
                                          [&envelopeGraph]() {
                                              return hbox({filler(),
                                                           envelopeGraph->Render(),
                                                           filler()});
                                          });

        auto attackSlider = Slider("Attack:", &controls.attack_pct, 1, 100, 1);
        auto decaySlider = Slider(" Decay:", &controls.decay_pct, 1, 100, 1);
        auto sustainSlider = Slider(" Sustain:", &controls.sustain_pct, 0, 100, 1);
        auto releaseSlider = Slider(" Release:", &controls.release_pct, 1, 100, 1);
        auto envelopeControlsContainer = Container::Horizontal({attackSlider, decaySlider, sustainSlider, releaseSlider});

        auto envelopeControls = Renderer(envelopeControlsContainer,
                                         [&attackSlider, &decaySlider, &sustainSlider, &releaseSlider]() {
                                             return hbox({attackSlider->Render(),
                                                          decaySlider->Render(),
                                                          sustainSlider->Render(),
                                                          releaseSlider->Render()});
                                         });

        auto envelopeAndControlsContainer = Container::Vertical({envelopeGraph, envelopeControls});
        auto envelopeTab = Renderer(envelopeAndControlsContainer, [&] {
            return vbox({envelopeContainer->Render() | flex,
                         envelopeControls->Render()}) |
                   borderRounded | color(Color::BlueLight);
        });


        // =========== Effects tab ===========
        auto delayGainSlider = Slider("Delay gain:", &controls.delayGain, 0., 1., .05);
        auto delaySlider = Slider("Delay (ms):", &controls.delay_ms, 1, 1000, 50);
        auto delayContainer = Container::Horizontal({delaySlider, delayGainSlider});
        auto delayControls = Renderer(delayContainer,
                                      [&delayGainSlider, &delaySlider]() {
                                          return hbox({delayGainSlider->Render(), delaySlider->Render()});
                                      });

        auto effectsSpacer = Renderer([&] {
            auto width = 80;
            auto c = Canvas(width, graphHeight);
            return vbox({canvas(std::move(c)) | hcenter});
        });

        auto effectsContainer = Container::Vertical({delayControls, effectsSpacer});
        auto effectsTab = Renderer(effectsContainer, [&] {
            return vbox({delayControls->Render(),
                         hbox({filler(),
                               effectsSpacer->Render(),
                               filler()})}) |
                   borderRounded | color(Color::BlueLight);
        });

        // =========== Tab Bar ===========
        auto tab_index = 0;
        auto tab_entries = std::vector<std::string>{"oscillators", "envelope", "effects"};
        auto selectedTab = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
        auto tabContent = Container::Tab({oscillatorsTab, envelopeTab, effectsTab}, &tab_index);

        // =========== Piano Roll ===========
        auto basePianoRoll = [](int width, int height) {
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

        auto activeNotes = [&](int width, [[maybe_unused]] int height) {
            std::vector<int> output(width, -1);
            for (auto i = 0; i < std::min(width, static_cast<int>(_keyboard.audibleNotes.size())); ++i) {
                if (_keyboard.audibleNotes[i].isOn()) {
                    output[i] = 1;
                }
            }
            return output;
        };

        auto pianoRoll = Renderer([&activeNotes, &basePianoRoll]() {
            return hbox({filler(),
                         vbox({graph(std::ref(activeNotes)) | size(HEIGHT, EQUAL, 2) | color(Color::GreenLight),
                               graph(std::ref(basePianoRoll)) | size(HEIGHT, EQUAL, 2) | color(Color::Default)}) |
                             size(WIDTH, EQUAL, 66),
                         filler()}) |
                   borderRounded | color(Color::RedLight);
        });

        auto mainContents = Container::Vertical({info,
                                                 selectedTab,
                                                 tabContent,
                                                 pianoRoll});

        auto mainRenderer = Renderer(mainContents, [&] {
            Element document = vbox({text("microtone") | bold | hcenter,
                                     selectedTab->Render(),
                                     tabContent->Render(),
                                     pianoRoll->Render()});

            if (showInfo) {
                document = dbox({
                    document,
                    info->Render() | clear_under | center,
                });
            }
            return document;
        });

        auto eventListener = CatchEvent(mainRenderer, [&](const Event& event) {
            if (event == Event::Character('q')) {
                onAboutToQuit();
                _screen.ExitLoopClosure()();
                return true;
            }
            if (event == Event::Return) {
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

Asciiboard::Asciiboard() :
    _impl{std::make_unique<impl>()} {};

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

void Asciiboard::loop(const SynthControls& initialControls, const OnControlsChangedFn& onControlsChangedFn, const OnAboutToQuitFn& onAboutToQuitFn) {
    _impl->loop(initialControls, onControlsChangedFn, onAboutToQuitFn);
}

Asciiboard::~Asciiboard() = default;

}
