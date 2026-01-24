#pragma once

#include <ftxui/component/component.hpp>

namespace asciiboard {

class OscillatorControls {
public:
    explicit OscillatorControls(const std::shared_ptr<State>& controls) :
        _controls(controls) {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;

        auto sineSlider = Slider("Sine:", &_controls->sineWeight_pct, 0, 100, 1);
        auto squareSlider = Slider(" Square:", &_controls->squareWeight_pct, 0, 100, 1);
        auto triangleSlider = Slider(" Triangle:", &_controls->triangleWeight_pct, 0, 100, 1);

        auto oscillatorMixerContainer = Container::Horizontal({
            sineSlider,
            squareSlider,
            triangleSlider,
        });

        auto oscillatorMixer = Renderer(oscillatorMixerContainer,
                                        [=, this] {
                                            return hbox({sineSlider->Render(),
                                                         squareSlider->Render(),
                                                         triangleSlider->Render()});
                                        });

        // Gain and LFO Frequency
        auto gainSlider = Slider("Gain:", &_controls->gain, .1, 1., .1);
        auto lfoFrequencyHzSlider = Slider("LFO Frequency (Hz):", &_controls->lfoFrequency_Hz, .01, 20, .1);
        auto lfoGainSlider = Slider("LFO Gain:", &_controls->lfoGain, 0., 1., .1);
        auto gainAndLfoControlsContainer = Container::Horizontal({gainSlider, lfoFrequencyHzSlider, lfoGainSlider});
        auto gainAndLfoControls = Renderer(gainAndLfoControlsContainer,
                                           [=] {
                                               return hbox({gainSlider->Render(),
                                                            lfoFrequencyHzSlider->Render(),
                                                            lfoGainSlider->Render()});
                                           });

        return Container::Vertical({oscillatorMixer, gainAndLfoControlsContainer});
    }

private:
    std::shared_ptr<State> _controls;
};

}
