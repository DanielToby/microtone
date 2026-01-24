#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/state.hpp"

namespace asciiboard {

class EnvelopeControls {
public:
    explicit EnvelopeControls(const std::shared_ptr<State>& controls) :
        _controls(controls) {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;

        auto attackSlider = Slider("Attack:", &_controls->attack_pct, 1, 100, 1);
        auto decaySlider = Slider(" Decay:", &_controls->decay_pct, 1, 100, 1);
        auto sustainSlider = Slider(" Sustain:", &_controls->sustain_pct, 0, 100, 1);
        auto releaseSlider = Slider(" Release:", &_controls->release_pct, 1, 100, 1);
        auto envelopeControlsContainer = Container::Horizontal({attackSlider, decaySlider, sustainSlider, releaseSlider});

        return Renderer(envelopeControlsContainer,
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