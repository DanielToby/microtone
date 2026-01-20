#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/synth_controls.hpp"

namespace asciiboard {

class EnvelopeControls {
public:
    explicit EnvelopeControls(const std::shared_ptr<SynthControls>& controls) :
        _controls(controls) {
        using namespace ftxui;

        _attackSlider = Slider("Attack:", &controls->attack_pct, 1, 100, 1);
        _decaySlider = Slider(" Decay:", &controls->decay_pct, 1, 100, 1);
        _sustainSlider = Slider(" Sustain:", &controls->sustain_pct, 0, 100, 1);
        _releaseSlider = Slider(" Release:", &controls->release_pct, 1, 100, 1);
        _envelopeControlsContainer = Container::Horizontal({_attackSlider, _decaySlider, _sustainSlider, _releaseSlider});

        _component = Renderer(_envelopeControlsContainer,
                              [this]() {
                                  return hbox({_attackSlider->Render(),
                                               _decaySlider->Render(),
                                               _sustainSlider->Render(),
                                               _releaseSlider->Render()});
                              });
    }

    [[nodiscard]] const ftxui::Component& component() const {
        return _component;
    }

private:
    ftxui::Component _component;

    ftxui::Component _attackSlider;
    ftxui::Component _decaySlider;
    ftxui::Component _sustainSlider;
    ftxui::Component _releaseSlider;
    ftxui::Component _envelopeControlsContainer;

    std::shared_ptr<SynthControls> _controls;
};

}