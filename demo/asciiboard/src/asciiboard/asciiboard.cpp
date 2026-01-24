#include <asciiboard/asciiboard.hpp>

#include <fmt/format.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include "asciiboard/components/effects_controls.hpp"
#include "asciiboard/components/envelope_controls.hpp"
#include "asciiboard/components/envelope_graph.hpp"
#include "asciiboard/components/info_message.hpp"
#include "asciiboard/components/oscillator_controls.hpp"
#include "asciiboard/components/oscilloscope.hpp"
#include "asciiboard/components/piano_roll.hpp"
#include "common/log.hpp"

#include <string>

namespace asciiboard {

using namespace ftxui;

namespace {

constexpr auto graphHeight = 120;
constexpr auto graphWidth = 600;
constexpr auto envelopeGraphWidth = 80;
constexpr auto effectsTabWidth = 80;

}

class Asciiboard::impl {
public:
    explicit impl(const State& initialControls, double sampleRate) :
        _screen{ScreenInteractive::Fullscreen()},
        _controls{std::make_unique<State>(initialControls)},
        _oscilloscope(sampleRate, graphWidth, graphHeight, _controls) {}

    void addOutputData(const common::audio::FrameBlock& audioBlock) {
        _oscilloscope.addAudioBlock(audioBlock);
        _screen.PostEvent(Event::Custom);
    }

    void updateMidiKeyboard(const common::midi::Keyboard& keyboard) {
        _pianoRoll.setMidiKeyboard(keyboard);
        _screen.PostEvent(Event::Custom);
    }

    void loop(const OnControlsChangedFn& onControlsChangedFn, const OnAboutToQuitFn& onAboutToQuit) {
        // =========== Info ===========
        auto infoState = InfoMessage{"Press <enter> to submit changes to the controls. Press 'q' to quit.",
                                     fmt::format("Logs are written to: {}", common::Log::getDefaultLogfilePath()),
                                     _controls};
        auto info = infoState.component();

        // =========== Oscillators tab ===========
        auto oscillatorControlsState = OscillatorControls{_controls};
        auto oscillatorControls = oscillatorControlsState.component();
        auto oscilloscope = _oscilloscope.component();
        auto oscillatorsContainer = Container::Vertical({oscilloscope, oscillatorControls});
        auto oscillatorsTab = Renderer(oscillatorsContainer, [&] {
            return vbox({
                       oscilloscope->Render() | flex,
                       oscillatorControls->Render(),
                   }) |
                   borderRounded | hcenter | color(Color::BlueLight);
        });

        // =========== Envelope tab ===========
        auto envelopeGraphState = EnvelopeGraph(envelopeGraphWidth, graphHeight, _controls);
        auto envelopeGraph = envelopeGraphState.component();
        auto envelopeControlsState = EnvelopeControls(_controls);
        auto envelopeControls = envelopeControlsState.component();
        auto envelopeAndControlsContainer = Container::Vertical({envelopeGraph, envelopeControls});
        auto envelopeTab = Renderer(envelopeAndControlsContainer, [&] {
            return vbox({envelopeGraph->Render() | flex,
                         envelopeControls->Render()}) |
                   borderRounded | color(Color::BlueLight);
        });

        // =========== Effects tab ===========
        auto effectsState = EffectsControls(effectsTabWidth, graphHeight, _controls);
        auto effectsTab = effectsState.component();

        // =========== Tab Bar ===========
        auto tabEntries = std::vector<std::string>{"oscillators", "envelope", "effects"};
        auto tabBar = Menu(&tabEntries, &_controls->selectedTab, MenuOption::HorizontalAnimated());
        auto tabContent = Container::Tab({oscillatorsTab, envelopeTab, effectsTab}, &_controls->selectedTab);

        // =========== Piano Roll ===========
        auto pianoRoll = _pianoRoll.component();

        auto mainContents = Container::Vertical({info,
                                                 tabBar,
                                                 tabContent,
                                                 pianoRoll});

        auto mainRenderer = Renderer(mainContents, [&] {
            Element document = vbox({text("microtone") | bold | hcenter,
                                     tabBar->Render(),
                                     tabContent->Render(),
                                     pianoRoll->Render()});

            if (_controls->showInfoMessage) {
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
                onControlsChangedFn(*_controls);
            }
            return false;
        });

        _screen.Loop(eventListener);
    }

    ScreenInteractive _screen;
    std::shared_ptr<State> _controls;
    Oscilloscope _oscilloscope;
    PianoRoll _pianoRoll;
};

Asciiboard::Asciiboard(const State& initialControls, double sampleRate) :
    _impl{std::make_unique<impl>(initialControls, sampleRate)} {};

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

void Asciiboard::loop(const OnControlsChangedFn& onControlsChangedFn, const OnAboutToQuitFn& onAboutToQuitFn) {
    _impl->loop(onControlsChangedFn, onAboutToQuitFn);
}

Asciiboard::~Asciiboard() = default;

}
