#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/state.hpp"
#include "asciiboard/components/graph.hpp"

namespace asciiboard {

class EnvelopeGraph {
public:
    EnvelopeGraph(int width, int height, const std::shared_ptr<State>& controls) :
        _width(width),
        _height(height),
        _controls(controls),
        _graph{_width, _height, getTitle(), getColors()},
        _lastUsedAdsr(_controls->getAdsr()) {}

    [[nodiscard]] ftxui::Component component() const {
        return _graph.component([this] {
            return getPoints(_controls->getAdsr(), _width, _height);
        });
    }

private:
    [[nodiscard]] static std::string getTitle() {
        return "Envelope (ms)";
    }

    [[nodiscard]] static std::vector<Point2D> getPoints(const synth::ADSR& adsr, int graphWidth, int graphHeight) {
        // A, D, S, and R are all [0, 1].
        // A, D, and R refer to millisecond values (x-axis).
        // S is intensity (y-axis).
        constexpr auto maxWidth = 4.;
        constexpr auto maxHeight = 1.;

        // Segment widths
        auto sustainWidth = maxWidth - adsr.attack - adsr.decay - adsr.release;

        constexpr auto attackHeight = 0.;
        auto sustainHeight = (1. - adsr.sustain) * maxHeight;

        auto lerp = [](double in, const std::array<double, 2>& inRange, const std::array<double, 2>& outRange) {
            return in * (outRange[1] - outRange[0]) / (inRange[1] - inRange[0]);
        };

        // Adds a 4 pixel margin around the graph, so values are in range [4, graphWidth - 4], [4, graphHeight - 4]
        constexpr auto margin = 4.;
        auto toGraphX = [&lerp, &maxWidth, &graphWidth, &margin](double x) {
            return static_cast<int>(lerp(x, {0., maxWidth}, {margin, graphWidth - margin}));
        };
        auto toGraphY = [&lerp, &maxHeight, &graphHeight, &margin](double y) {
            return static_cast<int>(lerp(y, {0., maxHeight}, {margin, graphHeight - margin}));
        };

        return {
            Point2D{toGraphX(0), toGraphY(maxHeight)},
            Point2D{toGraphX(adsr.attack), toGraphY(attackHeight)},
            Point2D{toGraphX(adsr.attack + adsr.decay), toGraphY(sustainHeight)},
            Point2D{toGraphX(adsr.attack + adsr.decay + sustainWidth), toGraphY(sustainHeight)},
            Point2D{toGraphX(maxWidth), toGraphY(maxHeight)},
        };
    }

    [[nodiscard]] static std::vector<ftxui::Color> getColors() {
        return {
            ftxui::Color::Cyan,
            ftxui::Color::BlueLight,
            ftxui::Color::Purple,
            ftxui::Color::Red};
    }

    int _width{200};
    int _height{200};
    std::shared_ptr<State> _controls;
    Graph _graph;

    // Drawing is cached based on this.
    synth::ADSR _lastUsedAdsr;
};

}