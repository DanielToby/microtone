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
    explicit impl(const SynthControls& initialControls, double sampleRate) :
        _screen{ScreenInteractive::Fullscreen()},
        _controls{std::make_unique<SynthControls>(initialControls)},
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
        auto info = InfoMessage{"Press <enter> to submit changes to the controls. Press 'q' to quit.",
                                fmt::format("Logs are written to: {}", common::Log::getDefaultLogfilePath())};

        // =========== Oscillators tab ===========
        auto oscillatorControls = OscillatorControls{_controls};
        auto oscillatorsContainer = Container::Vertical({_oscilloscope.component(), oscillatorControls.component()});
        auto oscillatorsTab = Renderer(oscillatorsContainer, [&] {
            return vbox({
                       _oscilloscope.component()->Render() | flex,
                       oscillatorControls.component()->Render(),
                   }) |
                   borderRounded | hcenter | color(Color::BlueLight);
        });

        // =========== Envelope tab ===========
        auto envelopeGraph = EnvelopeGraph(envelopeGraphWidth, graphHeight, _controls);
        auto envelopeControls = EnvelopeControls(_controls);
        auto envelopeAndControlsContainer = Container::Vertical({envelopeGraph.component(), envelopeControls.component()});
        auto envelopeTab = Renderer(envelopeAndControlsContainer, [&] {
            return vbox({envelopeGraph.component()->Render() | flex,
                         envelopeControls.component()->Render()}) |
                   borderRounded | color(Color::BlueLight);
        });

        // =========== Effects tab ===========
        auto effectsTab = EffectsControls(effectsTabWidth, graphHeight, _controls);

        // =========== Tab Bar ===========
        auto tab_index = 0;
        auto tab_entries = std::vector<std::string>{"oscillators", "envelope", "effects"};
        auto selectedTab = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
        auto tabContent = Container::Tab({oscillatorsTab, envelopeTab, effectsTab.component()}, &tab_index);

        auto mainContents = Container::Vertical({info.component(),
                                                 selectedTab,
                                                 tabContent,
                                                 _pianoRoll.component()});

        auto mainRenderer = Renderer(mainContents, [&] {
            Element document = vbox({text("microtone") | bold | hcenter,
                                     selectedTab->Render(),
                                     tabContent->Render(),
                                     _pianoRoll.component()->Render()});

            if (info.show()) {
                document = dbox({
                    document,
                    info.component()->Render() | clear_under | center,
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
    std::shared_ptr<SynthControls> _controls;
    Oscilloscope _oscilloscope;
    PianoRoll _pianoRoll;
};

Asciiboard::Asciiboard(const SynthControls& initialControls, double sampleRate) :
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
