#pragma once

#include <ftxui/component/component.hpp>

#include <asciiboard/state.hpp>

namespace asciiboard {

struct SelectableOscillatorControls : I_UIControls {
    explicit SelectableOscillatorControls(const std::shared_ptr<State>& state) :
        _state(state),
        _controls{
            _state->selectedOscillatorControl,
            {&_state->gain,
             &_state->sineWeight,
             &_state->squareWeight,
             &_state->triangleWeight,
             &_state->lfoFrequency,
             &state->lfoGain}} {}

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
    std::shared_ptr<State> _state; //< Stored to attach lifetime to that of _controls.
    SelectionInRange<std::vector<I_Incrementable*>> _controls;
};

[[nodiscard]] inline ftxui::Component makeOscillatorControls(const std::shared_ptr<State>& controls) {
    using namespace ftxui;

    auto gainSlider = Slider(
        "Master Gain:",
        &controls->gain.value,
        controls->gain.min,
        controls->gain.max,
        controls->gain.incrementAmount);

    auto sineSlider = Slider(
        "Sine:",
        &controls->sineWeight.value,
        controls->sineWeight.min,
        controls->sineWeight.max,
        controls->sineWeight.incrementAmount);

    auto squareSlider = Slider(
        "Square:",
        &controls->squareWeight.value,
        controls->squareWeight.min,
        controls->squareWeight.max,
        controls->squareWeight.incrementAmount);

    auto triangleSlider = Slider(
        "Triangle:",
        &controls->triangleWeight.value,
        controls->triangleWeight.min,
        controls->triangleWeight.max,
        controls->triangleWeight.incrementAmount);

    auto lfoFrequencyHzSlider = Slider(
        "LFO Frequency (Hz):",
        &controls->lfoFrequency.value,
        controls->lfoFrequency.min,
        controls->lfoFrequency.max,
        controls->lfoFrequency.incrementAmount);

    auto lfoGainSlider = Slider(
        "LFO Gain:",
        &controls->lfoGain.value,
        controls->lfoGain.min,
        controls->lfoGain.max,
        controls->lfoGain.incrementAmount);

    auto controlsContainer = Container::Tab({gainSlider,
                                             sineSlider,
                                             squareSlider,
                                             triangleSlider,
                                             lfoFrequencyHzSlider,
                                             lfoGainSlider},
                                            controls->selectedOscillatorControl.get());

    return Renderer(controlsContainer, [=] {
        return vbox({hbox({gainSlider->Render(),
                           filler() | size(WIDTH, EQUAL, 1),
                           sineSlider->Render(),
                           filler() | size(WIDTH, EQUAL, 1),
                           squareSlider->Render(),
                           filler() | size(WIDTH, EQUAL, 1),
                           triangleSlider->Render()}),
                     filler() | size(HEIGHT, EQUAL, 1),
                     hbox({lfoFrequencyHzSlider->Render(), lfoGainSlider->Render()})});
    });
}

}
