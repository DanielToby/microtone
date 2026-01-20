#pragma once

#include <ftxui/component/component.hpp>

namespace asciiboard {

class OscillatorControls {
public:
    explicit OscillatorControls(const std::shared_ptr<SynthControls>& controls) :
        _controls(controls) {
        using namespace ftxui;

        _sineSlider = Slider("Sine:", &_controls->sineWeight_pct, 0, 100, 1);
        _squareSlider = Slider(" Square:", &_controls->squareWeight_pct, 0, 100, 1);
        _triangleSlider = Slider(" Triangle:", &_controls->triangleWeight_pct, 0, 100, 1);

        _oscillatorMixerContainer = Container::Horizontal({
            _sineSlider,
            _squareSlider,
            _triangleSlider,
        });

        _oscillatorMixer = Renderer(_oscillatorMixerContainer,
                                    [this]() {
                                        return hbox({_sineSlider->Render(),
                                                     _squareSlider->Render(),
                                                     _triangleSlider->Render()});
                                    });

        // Gain and LFO Frequency
        _gainSlider = Slider("Gain:", &_controls->gain, .1, 1., .1);
        _lfoFrequencyHzSlider = Slider("LFO Frequency (Hz):", &_controls->lfoFrequency_Hz, .01, 20, .1);
        _lfoGainSlider = Slider("LFO Gain:", &_controls->lfoGain, 0., 1., .1);
        _gainAndLfoControlsContainer = Container::Horizontal({_gainSlider, _lfoFrequencyHzSlider, _lfoGainSlider});
        _gainAndLfoControls = Renderer(_gainAndLfoControlsContainer,
                                       [this]() {
                                           return hbox({_gainSlider->Render(),
                                                        _lfoFrequencyHzSlider->Render(),
                                                        _lfoGainSlider->Render()});
                                       });

        _component = Container::Vertical({_oscillatorMixer, _gainAndLfoControlsContainer});
    }

    [[nodiscard]] const ftxui::Component& component() const {
        return _component;
    }

private:
    ftxui::Component _component;
    std::shared_ptr<SynthControls> _controls;

    ftxui::Component _sineSlider;
    ftxui::Component _squareSlider;
    ftxui::Component _triangleSlider;
    ftxui::Component _oscillatorMixerContainer;
    ftxui::Component _oscillatorMixer;

    ftxui::Component _gainSlider;
    ftxui::Component _lfoFrequencyHzSlider;
    ftxui::Component _lfoGainSlider;
    ftxui::Component _gainAndLfoControlsContainer;
    ftxui::Component _gainAndLfoControls;
};

}
