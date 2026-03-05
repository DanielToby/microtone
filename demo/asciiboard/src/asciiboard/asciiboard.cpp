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
#include "asciiboard/components/compact_piano_roll.hpp"
#include "common/log.hpp"

#include <string>

namespace asciiboard {

using namespace ftxui;

namespace {

constexpr auto graphHeight = 60;
constexpr auto graphWidth = 180;
constexpr auto envelopeGraphWidth = 180;
constexpr auto effectsTabWidth = 80;

}

class Asciiboard::impl {
    [[nodiscard]] static Event quitEvent() {
        return Event::Character('q');
    }
    [[nodiscard]] static Event toggleInfoMessageEvent() {
        return Event::Return;
    }
    [[nodiscard]] static Event nextTabEvent() {
        return Event::Tab;
    }
    [[nodiscard]] static Event previousTabEvent() {
        return Event::TabReverse;
    }
    [[nodiscard]] static Event nextControlEvent() {
        return Event::ArrowDown;
    }
    [[nodiscard]] static Event previousControlEvent() {
        return Event::ArrowUp;
    }
    [[nodiscard]] static Event incrementValueEvent() {
        return Event::ArrowRight;
    }
    [[nodiscard]] static Event decrementValueEvent() {
        return Event::ArrowLeft;
    }

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

    void toggleInfoMessage() {
        _screen.PostEvent(toggleInfoMessageEvent());
    }

    void nextTab() {
        _screen.PostEvent(nextTabEvent());
    }

    void previousTab() {
        _screen.PostEvent(previousTabEvent());
    }

    void nextControl() {
        _screen.PostEvent(nextControlEvent());
    }

    void previousControl() {
        _screen.PostEvent(previousControlEvent());
    }

    void incrementValue() {
        _screen.PostEvent(incrementValueEvent());
    }

    void decrementValue() {
        _screen.PostEvent(decrementValueEvent());
    }

    void loop(const OnControlsChangedFn& onControlsChangedFn, const OnAboutToQuitFn& onAboutToQuit) {
        // =========== Info ===========
        auto infoState = InfoMessage{"Press <enter> to submit changes to the controls. Press 'q' to quit.",
                                     fmt::format("Logs are written to: {}", common::Log::getDefaultLogfilePath()),
                                     _controls};
        auto info = infoState.component();

        // =========== Oscillators tab ===========
        auto oscillatorControls = makeOscillatorControls(_controls);
        auto selectableOscillatorControls = SelectableOscillatorControls(_controls);
        auto oscilloscope = _oscilloscope.component();
        auto oscillatorsTab = Renderer(oscillatorControls, [&] {
            return vbox({
                       oscilloscope->Render() | flex,
                       oscillatorControls->Render(),
                   }) |
                   borderRounded | color(Color::BlueLight);
        });

        // =========== Envelope tab ===========
        auto envelopeGraphState = EnvelopeGraph(envelopeGraphWidth, graphHeight, _controls);
        auto envelopeGraph = envelopeGraphState.component();
        auto envelopeControlsState = EnvelopeControls(_controls);
        auto envelopeControls = envelopeControlsState.component();
        auto selectableEnvelopeControls = SelectableEnvelopeControls(_controls);
        auto envelopeTab = Renderer(envelopeControls, [&] {
            return vbox({
                        envelopeGraph->Render() | flex,
                        envelopeControls->Render()
                   }) |
                   borderRounded | color(Color::BlueLight);
        });

        // =========== Effects tab ===========
        auto effectsState = EffectsControls(effectsTabWidth, graphHeight, _controls);
        auto selectableEffectsControls = SelectableEffectsControls(_controls);
        auto effectsTab = effectsState.component();

        // =========== Tab Bar ===========
        auto tabEntries = std::vector<std::string>{"oscillators", "envelope", "effects"};
        auto tabOption = MenuOption::Horizontal();
        tabOption.underline.enabled = false;
        tabOption.entries_option.transform = [](EntryState state) {
            Element e = text(state.label);
            if (state.active) {
                e = e | bold | underlined;
            }
            return e;
        };

        auto tabBar = Menu(&tabEntries, _controls->selectedTab.get(), tabOption);
        auto tabContent = Container::Tab({oscillatorsTab, envelopeTab, effectsTab}, _controls->selectedTab.get());

        // =========== Piano Roll ===========
        auto pianoRollComponent =_pianoRoll.component();
        auto pianoRoll = Renderer(pianoRollComponent, [=] {
            return vbox({
                        pianoRollComponent->Render() | flex,
                   }) | borderRounded | color(Color::BlueLight);
        });

        auto mainRenderer = Renderer(tabContent, [&] {
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

        auto getCurrentControls = [&]() -> I_UIControls* {
            switch (*_controls->selectedTab) {
            case 0:
                return &selectableOscillatorControls;
            case 1:
                return &selectableEnvelopeControls;
            case 2:
                return &selectableEffectsControls;
            default:
                throw std::runtime_error("Invalid selectedPage");
            }
        };

        auto eventListener = CatchEvent(mainRenderer, [&](const Event& event) {
            if (event == quitEvent()) {
                onAboutToQuit();
                _screen.ExitLoopClosure()();
                return true;
            }
            if (event == toggleInfoMessageEvent()) {
                _controls->showInfoMessage = !_controls->showInfoMessage;
                return true;
            }
            if (event == nextTabEvent()) {
                if (*_controls->selectedTab < tabEntries.size() - 1) {
                    (*_controls->selectedTab)++;
                }
                _controls->resetControlSelections();
                return true;
            }
            if (event == previousTabEvent()) {
                if (*_controls->selectedTab > 0) {
                    (*_controls->selectedTab)--;
                }
                _controls->resetControlSelections();
                return true;
            }
            if (event == nextControlEvent()) {
                getCurrentControls()->nextControl();
                return true;
            }
            if (event == previousControlEvent()) {
                getCurrentControls()->previousControl();
                return true;
            }
            if (event == incrementValueEvent()) {
                getCurrentControls()->currentControl().increment();
                onControlsChangedFn(*_controls);
                return true;
            }
            if (event == decrementValueEvent()) {
                getCurrentControls()->currentControl().decrement();
                onControlsChangedFn(*_controls);
                return true;
            }
            return false;
        });

        _screen.Loop(eventListener);
    }

    ScreenInteractive _screen;
    std::shared_ptr<State> _controls;
    Oscilloscope _oscilloscope;
    CompactPianoRoll _pianoRoll;
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

void Asciiboard::toggleInfoMessage() {
    _impl->toggleInfoMessage();
}

void Asciiboard::nextTab() {
    _impl->nextTab();
}

void Asciiboard::previousTab() {
    _impl->previousTab();
}

void Asciiboard::nextControl() {
    _impl->nextControl();
}

void Asciiboard::previousControl() {
    _impl->previousControl();
}

void Asciiboard::incrementValue() {
    _impl->incrementValue();
}

void Asciiboard::decrementValue() {
    _impl->decrementValue();
}

void Asciiboard::loop(const OnControlsChangedFn& onControlsChangedFn, const OnAboutToQuitFn& onAboutToQuitFn) {
    _impl->loop(onControlsChangedFn, onAboutToQuitFn);
}

Asciiboard::~Asciiboard() = default;

}
