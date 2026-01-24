#pragma once

#include <ftxui/component/component.hpp>

#include <deque>

namespace asciiboard {

namespace detail {

constexpr auto scaleFactors = std::array{
    .25f,
    .5f,
    1.0f,
    2.0f,
    5.0f};

constexpr auto blocksToShow = std::array{
    1,
    2,
    5,
    10,
    20};

template <typename Range>
[[nodiscard]] std::vector<std::string> toScaleFactorStrings(const Range& values) {
    // TODO: C++23 std::ranges::to
    std::vector<std::string> result;
    result.reserve(values.size());
    for (const auto& v : values) {
        result.push_back(fmt::format("{}x", v));
    }
    return result;
}

[[nodiscard]] inline double toMilliseconds(double numBlocks, double sampleRate) {
    return numBlocks * common::audio::AudioBlockSize / sampleRate * 1000;
}

template <typename Range>
[[nodiscard]] std::vector<std::string> toMillisecondStrings(const Range& values, double sampleRate) {
    // TODO: C++23 std::ranges::to
    std::vector<std::string> result;
    result.reserve(values.size());
    for (const auto& v : values) {
        result.push_back(fmt::format("{}ms", static_cast<int>(toMilliseconds(v, sampleRate))));
    }
    return result;
}

}

class Oscilloscope {
public:
    Oscilloscope(double sampleRate, int graphWidth, int graphHeight, const std::shared_ptr<State>& controls) :
        _sampleRate(sampleRate),
        _graphHeight(graphHeight),
        _graphWidth(graphWidth),
        _controls(controls),
        _scaleFactorStrings(detail::toScaleFactorStrings(detail::scaleFactors)),
        _millisecondStrings(detail::toMillisecondStrings(detail::blocksToShow, _sampleRate)) {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;

        auto scaleFactorSelector = Toggle(&_scaleFactorStrings, &_controls->oscilloscopeScaleFactorIndex);
        auto timelineScaleSelector = Toggle(&_millisecondStrings, &_controls->oscilloscopeTimelineSizeIndex);
        auto oscilloscopeControlsContainer = Container::Horizontal({scaleFactorSelector, timelineScaleSelector});

        const auto getXValue = [this](std::size_t blockIndex, std::size_t i) {
            const auto totalSize = static_cast<double>(detail::blocksToShow[_controls->oscilloscopeTimelineSizeIndex] * common::audio::AudioBlockSize);
            auto index = static_cast<double>(blockIndex * common::audio::AudioBlockSize + i);
            return static_cast<int>(index / totalSize * _graphWidth);
        };
        const auto getYValue = [this](float y) {
            const auto scaledValue = y * detail::scaleFactors[_controls->oscilloscopeScaleFactorIndex];
            const auto totalSize = 2.f;                // [-1, 1].
            auto value = (scaledValue + 1) / totalSize;// [0, 1], may overflow from scale factor.
            if (std::abs(value) < 1e-5f) {
                value = 0.f;
            }
            return static_cast<int>(value * static_cast<float>(_graphHeight));
        };

        auto canvasComponent = Renderer([=, this] {
            auto c = Canvas(_graphWidth, _graphHeight);
            for (auto blockIndex = 0; blockIndex < _data.size(); ++blockIndex) {
                for (auto i = 0; i < common::audio::AudioBlockSize - 1; ++i) {
                    const auto x0 = getXValue(blockIndex, i);
                    const auto y0 = getYValue(_data.at(blockIndex).at(i));
                    const auto x1 = getXValue(blockIndex, i + 1);
                    const auto y1 = getYValue(_data.at(blockIndex).at(i + 1));

                    // Skip points that are out of range.
                    if ((y0 < 0 && y1 < 0) || (y0 > _graphHeight && y1 > _graphHeight)) {
                        continue;
                    }

                    c.DrawPointLine(x0,
                                    std::clamp(y0, 0, _graphHeight),
                                    x1,
                                    std::clamp(y1, 0, _graphHeight),
                                    Color::Purple);
                }
            }
            return canvas(c) | hcenter | flex;
        });

        auto container = Container::Vertical({oscilloscopeControlsContainer, canvasComponent});

        return Renderer(container, [=] {
            return vbox({
                hbox({
                    scaleFactorSelector->Render(),
                    filler() | size(WIDTH, EQUAL, 4),
                    timelineScaleSelector->Render(),
                }) | hcenter,

                vbox({
                    filler(),
                    canvasComponent->Render() | hcenter,
                    filler(),
                }) | flex,
            });
        });
    }

    void addAudioBlock(const common::audio::FrameBlock& block) {
        _data.push_back(block);
        if (_data.size() > detail::blocksToShow[_controls->oscilloscopeTimelineSizeIndex]) {
            _data.pop_front();
        }
    }

private:
    double _sampleRate{44100};
    int _graphHeight{100};
    int _graphWidth{100};
    std::shared_ptr<State> _controls;
    std::vector<std::string> _scaleFactorStrings;
    std::vector<std::string> _millisecondStrings;

    std::deque<common::audio::FrameBlock> _data;
};

}
