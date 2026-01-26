#pragma once

#include <ftxui/component/component.hpp>

#include "asciiboard/state.hpp"

namespace asciiboard {

struct Point2D {
    int x;
    int y;
};

//! Note: The top left corner is (0, 0)!
class Graph {
public:
    Graph(int width, int height, const std::string& title, const std::optional<std::vector<ftxui::Color>>& lineColors) :
        _width(width),
        _height(height),
        _title{title},
        _lineColors(lineColors) {}

    Graph& operator=(const Graph& other) {
        _width = other._width;
        _height = other._height;
        _title = other._title;
        _lineColors = other._lineColors;
        return *this;
    }

    [[nodiscard]] ftxui::Component component(const std::function<std::vector<Point2D>()>& getPoints) const {
        using namespace ftxui;

        auto graph = Renderer([getPoints, this] {
            auto c = Canvas(_width, _height);
            auto points = getPoints();
            throwIfInvalid(points);

            if (!points.empty()) {
                for (auto i = 0; i < points.size() - 1; ++i) {
                    const auto& p0 = points[i];
                    const auto& p1 = points[i + 1];
                    auto color = std::invoke([c = _lineColors, i]() -> Color {
                        if (c) {
                            return c->at(i);
                        }
                        return Color::Purple;
                    });
                    c.DrawPointLine(p0.x, p0.y, p1.x, p1.y, color);
                }
            }
            return vbox({text(_title) | hcenter, canvas(std::move(c)) | hcenter});
        });

        return Renderer(graph,
                        [=] {
                            return hbox({filler(),
                                         graph->Render(),
                                         filler()});
                        });
    }

    void throwIfInvalid(const std::vector<Point2D>& points) const {
        if (_lineColors && _lineColors->size() != points.size() - 1) {
            throw common::MicrotoneException("Invalid points.");
        }
        auto xInRange = [this](int x) { return 0 <= x && x <= _width; };
        auto yInRange = [this](int y) { return 0 <= y && y <= _height; };
        const auto bothInRange = [&](const Point2D& point) { return xInRange(point.x) && yInRange(point.y); };
        if (!std::all_of(points.begin(), points.end(), bothInRange)) {
            throw common::MicrotoneException("Invalid args.");
        }
    }

private:
    int _width{200};
    int _height{200};
    std::string _title;
    std::optional<std::vector<ftxui::Color>> _lineColors;
};

}