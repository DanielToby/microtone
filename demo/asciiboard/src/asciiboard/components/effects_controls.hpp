#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/state.hpp"

namespace asciiboard {

class EffectsControls {
public:
    explicit EffectsControls(int width, int height, const std::shared_ptr<State>& controls) :
        _width(width),
        _height(height),
        _controls(controls) {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;

        auto delayGainSlider = Slider("Delay gain:", &_controls->delayGain, 0., 1., .05);
        auto delaySlider = Slider("Delay (ms):", &_controls->delay_ms, 1, 1000, 50);
        auto delayContainer = Container::Horizontal({delaySlider, delayGainSlider});
        auto delayControls = Renderer(delayContainer,
                                      [=]() {
                                          return hbox({delayGainSlider->Render(), delaySlider->Render()});
                                      });

        auto lowPassCutoffSlider = Slider("Low Pass Cutoff (Hz):", &_controls->lowPassCutoffFrequency_Hz, 0.01, 5000., 250.);
        auto highPassCutoffSlider = Slider("High Pass Cutoff (Hz):", &_controls->highPassCutoffFrequency_Hz, 0.01, 5000., 250.);
        auto equalizerContainer = Container::Horizontal({lowPassCutoffSlider, highPassCutoffSlider});
        auto equalizerControls = Renderer(equalizerContainer,
                                          [=]() {
                                              return hbox({lowPassCutoffSlider->Render(), highPassCutoffSlider->Render()});
                                          });
                                      
        auto effectsSpacer = Renderer([this] {
            auto c = Canvas(_width, _height);
            return vbox({canvas(std::move(c)) | hcenter});
        });

        auto effectsContainer = Container::Vertical({delayControls, equalizerControls, effectsSpacer});
        return Renderer(effectsContainer, [=] {
            return vbox({delayControls->Render(),
                         equalizerControls->Render(),
                         hbox({filler(),
                               effectsSpacer->Render(),
                               filler()})}) |
                   borderRounded | color(Color::BlueLight);
        });
    }

private:
    int _width;
    int _height;
    std::shared_ptr<State> _controls;
};

}