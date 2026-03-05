#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/state.hpp"

namespace asciiboard {

struct SelectableEnvelopeControls : I_UIControls {
    explicit SelectableEnvelopeControls(const std::shared_ptr<State>& state) :
        _state(state),
        _controls{
            state->selectedOEnvelopeControl,
            {&_state->attack,
             &_state->decay,
             &_state->sustain,
             &_state->release}} {}

    [[nodiscard]] I_Incrementable& currentControl() override {
        return _controls.currentItem();
    }

    void nextControl() override {
        _controls.nextItem();
    }

    void previousControl() override {
        _controls.previousItem();
    }

private:
    std::shared_ptr<State> _state;//< Stored to attach lifetime to that of _controls.
    SelectionInRange<std::vector<I_Incrementable*>> _controls;
};

class EnvelopeControls {
public:
    explicit EnvelopeControls(const std::shared_ptr<State>& controls) :
        _controls(controls) {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;

        auto attackSlider = Slider(
            "Attack:",
            &_controls->attack.value,
            _controls->attack.min,
            _controls->attack.max,
            _controls->attack.incrementAmount);
        auto decaySlider = Slider(
            " Decay:",
            &_controls->decay.value,
            _controls->decay.min,
            _controls->decay.max,
            _controls->decay.incrementAmount);
        auto sustainSlider = Slider(
            " Sustain:",
            &_controls->sustain.value,
            _controls->sustain.min,
            _controls->sustain.max,
            _controls->sustain.incrementAmount);
        auto releaseSlider = Slider(
            " Release:",
            &_controls->release.value,
            _controls->release.min,
            _controls->release.max,
            _controls->release.incrementAmount);

        auto controlsContainer = Container::Tab({attackSlider,
                                                 decaySlider,
                                                 sustainSlider,
                                                 releaseSlider},
                                                _controls->selectedOEnvelopeControl.get());

        return Renderer(controlsContainer,
                        [=] {
                            return hbox({attackSlider->Render(),
                                         decaySlider->Render(),
                                         sustainSlider->Render(),
                                         releaseSlider->Render()});
                        });
    }

private:
    std::shared_ptr<State> _controls;
};

}