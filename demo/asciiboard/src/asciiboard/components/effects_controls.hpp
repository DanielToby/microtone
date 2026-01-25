#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/state.hpp"

namespace asciiboard {

class EffectsControls {
public:
    explicit EffectsControls(int width, int height, const std::shared_ptr<State>& controls) :
        _width(width),
        _height(height),
        _controls(controls),
        _filterStrings{"Low Pass", "High Pass"} {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;

        auto delayGainSlider = Slider("Delay gain:", &_controls->delayGain, 0., 1., .05);
        auto delaySlider = Slider("Delay (ms):", &_controls->delay_ms, 1, 1000, 50);
        auto delayContainer = Container::Vertical({delaySlider, delayGainSlider});
        auto delayControls = Renderer(delayContainer,
                                      [=]() {
                                          return vbox({
                                              text("Delay"),
                                              filler() | size(HEIGHT, EQUAL, 1),
                                              delayGainSlider->Render(),
                                              delaySlider->Render()
                                          });
                                      });

        auto filterType = Radiobox(_filterStrings, &_controls->filterTypeIndex);
        auto filterCutoffFrequency = Slider("Filter Cutoff (Hz):", &_controls->filterCutoffFrequencyHz, 0.01, 5000., 250.);
        auto filterLfoDepth = Slider("LFO Depth (Hz):", &_controls->filterLfoDepthHz, 0.01, 250., 20.);
        auto filterLfoRate = Slider("LFO Rate (Hz):", &_controls->filterLfoFrequencyHz, 0.25, 60., 5.);
        auto filterContainer = Container::Vertical({
            filterType,
            filterCutoffFrequency,
            filterLfoDepth,
            filterLfoRate
        });
        auto filterControls = Renderer(filterContainer,
                                          [=]() {
                                              return vbox({
                                                  text("Filter"),
                                                  filler() | size(HEIGHT, EQUAL, 1),
                                                  filterType->Render() | color(Color::GrayDark),
                                                  filterCutoffFrequency->Render(),
                                                  filterLfoDepth->Render(),
                                                  filterLfoRate->Render(),
                                              });
                                          });

        auto effectsContainer = Container::Vertical({delayControls, filterControls});
        return Renderer(effectsContainer, [=] {
            return vbox({delayControls->Render(),
                         filler() | size(HEIGHT, EQUAL, 2),
                         filterControls->Render()}) |
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