#pragma once

#include <ftxui/component/component.hpp>

namespace asciiboard {

class OscillatorControls {
public:
    explicit OscillatorControls(const std::shared_ptr<State>& controls) :
        _controls(controls) {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;

        auto gainSlider = Slider("Master Gain:", &_controls->gain, .1, 1., .1);
        auto sineSlider = Slider("Sine:", &_controls->sineWeight_pct, 0, 100, 1);
        auto squareSlider = Slider(" Square:", &_controls->squareWeight_pct, 0, 100, 1);
        auto triangleSlider = Slider("Triangle:", &_controls->triangleWeight_pct, 0, 100, 1);

        auto oscillatorMixerContainer = Container::Horizontal({
            gainSlider,
            sineSlider,
            squareSlider,
            triangleSlider,
        });

        auto oscillatorMixer = Renderer(oscillatorMixerContainer,
                                        [=] {
                                            return hbox({gainSlider->Render(),
                                                         filler() | size(WIDTH, EQUAL, 1),
                                                         sineSlider->Render(),
                                                         filler() | size(WIDTH, EQUAL, 1),
                                                         squareSlider->Render(),
                                                         filler() | size(WIDTH, EQUAL, 1),
                                                         triangleSlider->Render()});
                                        });

        // LFO Controls
        auto lfoFrequencyHzSlider = Slider("LFO Frequency (Hz):", &_controls->lfoFrequency_Hz, .01, 20, .1);
        auto lfoGainSlider = Slider("LFO Gain:", &_controls->lfoGain, 0., 1., .1);
        auto lfoControlsContainer = Container::Horizontal({lfoFrequencyHzSlider, lfoGainSlider});
        auto lfoControls = Renderer(lfoControlsContainer, [=] { return hbox({lfoFrequencyHzSlider->Render(), lfoGainSlider->Render()}); });

        auto container = Container::Vertical({oscillatorMixer, lfoControls});
        return Renderer(container, [=] {
            return vbox({
                oscillatorMixer->Render(),
                filler() | size(HEIGHT, EQUAL, 1),
                lfoControlsContainer->Render()
            });
        });
    }

private:
    std::shared_ptr<State> _controls;
};

}
