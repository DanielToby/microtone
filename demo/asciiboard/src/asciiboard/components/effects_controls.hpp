#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/synth_controls.hpp"

namespace asciiboard {

class EffectsControls {
public:
    explicit EffectsControls(int width, int height, const std::shared_ptr<SynthControls>& controls) :
        _width(width),
        _height(height),
        _controls(controls) {
        using namespace ftxui;

        _delayGainSlider = Slider("Delay gain:", &controls->delayGain, 0., 1., .05);
        _delaySlider = Slider("Delay (ms):", &controls->delay_ms, 1, 1000, 50);
        _delayContainer = Container::Horizontal({_delaySlider, _delayGainSlider});
        _delayControls = Renderer(_delayContainer,
                                  [this]() {
                                      return hbox({_delayGainSlider->Render(), _delaySlider->Render()});
                                  });

        _lowPassCutoffSlider = Slider("Low Pass Cutoff (Hz):", &controls->lowPassCutoffFrequency_Hz, 0.01, 5000., 250.);
        _highPassCutoffSlider = Slider("High Pass Cutoff (Hz):", &controls->highPassCutoffFrequency_Hz, 0.01, 5000., 250.);
        _equalizerContainer = Container::Horizontal({_lowPassCutoffSlider, _highPassCutoffSlider});
        _equalizerControls = Renderer(_equalizerContainer,
                                      [this]() {
                                          return hbox({_lowPassCutoffSlider->Render(), _highPassCutoffSlider->Render()});
                                      });

        _effectsSpacer = Renderer([this] {
            auto c = Canvas(_width, _height);
            return vbox({canvas(std::move(c)) | hcenter});
        });

        _effectsContainer = Container::Vertical({_delayControls, _equalizerControls, _effectsSpacer});
        _component = Renderer(_effectsContainer, [&] {
            return vbox({_delayControls->Render(),
                         _equalizerControls->Render(),
                         hbox({filler(),
                               _effectsSpacer->Render(),
                               filler()})}) |
                   borderRounded | color(Color::BlueLight);
        });
    }

    [[nodiscard]] const ftxui::Component& component() const {
        return _component;
    }

private:
    ftxui::Component _component;
    int _width;
    int _height;
    std::shared_ptr<SynthControls> _controls;

    ftxui::Component _delayGainSlider;
    ftxui::Component _delaySlider;
    ftxui::Component _delayContainer;
    ftxui::Component _delayControls;

    ftxui::Component _lowPassCutoffSlider;
    ftxui::Component _highPassCutoffSlider;
    ftxui::Component _equalizerContainer;
    ftxui::Component _equalizerControls;

    ftxui::Component _effectsSpacer;
    ftxui::Component _effectsContainer;
};

}