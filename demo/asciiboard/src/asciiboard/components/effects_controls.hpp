#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/state.hpp"

namespace asciiboard {

struct SelectableEffectsControls : I_UIControls {
    explicit SelectableEffectsControls(const std::shared_ptr<State>& state) :
        _state(state),
        _controls{
            _state->selectedEffectControl,
            {&_state->delayGain,
             &_state->delay_ms,
             &_state->filterTypeIndex,
             &_state->filterCutoffFrequency,
             &_state->filterLfoDepth,
             &_state->filterLfoFrequency}} {}

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

class EffectsControls {
public:
    explicit EffectsControls(int width, int height, const std::shared_ptr<State>& controls) :
        _width(width),
        _height(height),
        _controls(controls),
        _filterStrings{"Low Pass", "High Pass"} {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;

        auto delayGainSlider = Slider(
            "Delay gain:",
            &_controls->delayGain.value,
            _controls->delayGain.min,
            _controls->delayGain.max,
            _controls->delayGain.incrementAmount);

        auto delaySlider = Slider(
            "Delay (ms):",
            &_controls->delay_ms.value,
            _controls->delay_ms.min,
            _controls->delay_ms.max,
            _controls->delay_ms.incrementAmount);

        auto filterType = Radiobox(_filterStrings, &_controls->filterTypeIndex.value);

        auto filterCutoffFrequency = Slider(
            "Filter Cutoff (Hz):",
            &_controls->filterCutoffFrequency.value,
            _controls->filterCutoffFrequency.min,
            _controls->filterCutoffFrequency.max,
            _controls->filterCutoffFrequency.incrementAmount);

        auto filterLfoDepth = Slider(
            "LFO Depth (Hz):",
            &_controls->filterLfoDepth.value,
            _controls->filterLfoDepth.min,
            _controls->filterLfoDepth.max,
            _controls->filterLfoDepth.incrementAmount);

        auto filterLfoRate = Slider(
            "LFO Rate (Hz):",
            &_controls->filterLfoFrequency.value,
            _controls->filterLfoFrequency.min,
            _controls->filterLfoFrequency.max,
            _controls->filterLfoFrequency.incrementAmount);

        auto controlsContainer = Container::Tab({delayGainSlider,
                                                 delaySlider,
                                                 filterType,
                                                 filterCutoffFrequency,
                                                 filterLfoDepth,
                                                 filterLfoRate},
                                                _controls->selectedEffectControl.get());

        return Renderer(controlsContainer, [=] {
            return vbox({vbox({text("Delay"),
                               filler() | size(HEIGHT, EQUAL, 1),
                               delayGainSlider->Render(),
                               delaySlider->Render()}),
                         filler() | size(HEIGHT, EQUAL, 2),
                         vbox({
                             text("Filter"),
                             filler() | size(HEIGHT, EQUAL, 1),
                             filterType->Render() | color(Color::GrayDark),
                             filterCutoffFrequency->Render(),
                             filterLfoDepth->Render(),
                             filterLfoRate->Render(),
                         })}) |
                   borderRounded | color(Color::BlueLight);
        });
    }

private:
    int _width;
    int _height;
    std::shared_ptr<State> _controls;

    std::vector<std::string> _filterStrings;
};

}