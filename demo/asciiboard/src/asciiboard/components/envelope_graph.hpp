#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/synth_controls.hpp"

namespace asciiboard {

class EnvelopeGraph {
public:
    EnvelopeGraph(int width, int height, const std::shared_ptr<SynthControls>& controls) :
        _width(width),
        _height(height),
        _controls(controls) {
        using namespace ftxui;

        _graph = Renderer([this] {
            auto c = Canvas(_width, _height);
            auto effectiveHeight = _height - 5;
            /*
             *           / \
             *         /    \
             *       /       \___________
             *     /                     \
             *     0     x1  x2          x3
             *     a     d   s           r
             */

            // The "sustain" phase always has a width of w/4.
            const auto sustainWidth = _width / 4;

            // The remaining width is split among the other phases.
            const auto remainingWidth = (_width - sustainWidth);

            const auto totalTime = _controls->attack_pct + _controls->decay_pct + _controls->release_pct;
            const auto x1 = (static_cast<double>(_controls->attack_pct) / totalTime) * remainingWidth;
            const auto x2 = ((static_cast<double>(_controls->attack_pct) + _controls->decay_pct) / totalTime) * remainingWidth;
            const auto x3 = x2 + sustainWidth;

            const auto sustainHeight = effectiveHeight - (effectiveHeight * (static_cast<double>(_controls->sustain_pct) / 100));

            c.DrawPointLine(0, effectiveHeight, x1, 0, Color::Cyan);
            c.DrawPointLine(x1, 0, x2, sustainHeight, Color::BlueLight);
            c.DrawPointLine(x2, sustainHeight, x3, sustainHeight, Color::Purple);
            c.DrawPointLine(x3, sustainHeight, _width, effectiveHeight, Color::Red);

            return vbox({text("Envelope (ms)") | hcenter, canvas(std::move(c)) | hcenter});
        });

        _component = Renderer(_graph,
                              [this]() {
                                  return hbox({filler(),
                                               _graph->Render(),
                                               filler()});
                              });
    }

    [[nodiscard]] const ftxui::Component& component() const {
        return _component;
    }

private:
    ftxui::Component _graph;
    ftxui::Component _component;

    int _width{200};
    int _height{200};

    std::shared_ptr<SynthControls> _controls;
};

}