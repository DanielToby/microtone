#pragma once

#include <ftxui/component/component.hpp>

#include "common/dirty_flagged.hpp"
#include "common/sliding_window.hpp"

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

[[nodiscard]] inline int getXValueOnCanvas(std::size_t blockIndex, std::size_t i, std::size_t windowSize, int width) {
    const auto totalSize = static_cast<double>(windowSize * common::audio::AudioBlockSize);
    auto index = static_cast<double>(blockIndex * common::audio::AudioBlockSize + i);
    return static_cast<int>(index / totalSize * width);
};

[[nodiscard]] inline int getYValueOnCanvas(float y, float scaleFactor, int height) {
    auto value = (y * scaleFactor + 1) / 2.f; // [0, 1], may overflow from scale factor.
    if (std::abs(value) < 1e-5f) {
        value = 0.f;
    }
    return static_cast<int>(value * static_cast<float>(height));
};

[[nodiscard]] inline ftxui::Canvas makeCanvas(int width,
                                              int height,
                                              std::size_t windowSize,
                                              float scaleFactor,
                                              const std::vector<common::audio::FrameBlock>& blocks) {
    auto result = ftxui::Canvas(width, height);
    for (auto blockIndex = 0; blockIndex < blocks.size(); ++blockIndex) {
        for (auto i = 0; i < common::audio::AudioBlockSize - 1; ++i) {
            const auto x0 = getXValueOnCanvas(blockIndex, i, windowSize, width);
            const auto y0 = getYValueOnCanvas(blocks.at(blockIndex).at(i), scaleFactor, height);
            const auto x1 = getXValueOnCanvas(blockIndex, i + 1, windowSize, width);
            const auto y1 = getYValueOnCanvas(blocks.at(blockIndex).at(i + 1), scaleFactor, height);

            // Skip points that are out of range.
            if ((y0 < 0 && y1 < 0) || (y0 > height && y1 > height)) {
                continue;
            }

            result.DrawPointLine(x0,
                                 std::clamp(y0, 0, height),
                                 x1,
                                 std::clamp(y1, 0, height),
                                 ftxui::Color::Purple);
        }
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

    [[nodiscard]] ftxui::Component component() {
        using namespace ftxui;

        auto scaleFactorSelector = Toggle(&_scaleFactorStrings, &_controls->oscilloscopeScaleFactorIndex);
        auto timelineScaleSelector = Toggle(&_millisecondStrings, &_controls->oscilloscopeTimelineSizeIndex);
        auto toggleLive = Checkbox("Live", &_controls->isOscilloscopeLive);
        auto stepBack = Button("-10ms", [this] {
            auto& blockWindow = _blockWindow.getWritable();
            if (!_controls->isOscilloscopeLive && blockWindow.hasPrevious()) {
                blockWindow.stepBack();
            } }, ButtonOption::Ascii());
        auto stepForward = Button("+10ms", [this] {
            auto& blockWindow = _blockWindow.getWritable();
            if (!_controls->isOscilloscopeLive && blockWindow.hasNext()) {
                blockWindow.stepForward();
            } }, ButtonOption::Ascii());

        auto oscilloscopeControlsContainer = Container::Horizontal({scaleFactorSelector, timelineScaleSelector, toggleLive, stepBack, stepForward});

        auto canvasComponent = Renderer([=, this] {
            if (_blockWindow.dirty) {
                const auto windowSize = detail::blocksToShow[_controls->oscilloscopeTimelineSizeIndex];
                const auto currentWindow = _blockWindow.getReadable().currentWindow(windowSize);
                _canvas = detail::makeCanvas(
                    _graphWidth,
                    _graphHeight,
                    windowSize,
                    detail::scaleFactors[_controls->oscilloscopeScaleFactorIndex],
                    currentWindow);
            }
            return canvas(_canvas) | hcenter | flex;
        });

        auto container = Container::Vertical({oscilloscopeControlsContainer, canvasComponent});

        return Renderer(container, [=] {
            return vbox({
                hbox({
                    scaleFactorSelector->Render(),
                    filler() | size(WIDTH, EQUAL, 4),
                    timelineScaleSelector->Render(),
                    filler() | size(WIDTH, EQUAL, 4),
                    toggleLive->Render(),
                    filler() | size(WIDTH, EQUAL, 1),
                    stepBack->Render(),
                    stepForward->Render(),
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
        if (_controls->isOscilloscopeLive) {
            auto& blockWindow = _blockWindow.getWritable();
            blockWindow.push(block);
            const auto windowSize = detail::blocksToShow[_controls->oscilloscopeTimelineSizeIndex];
            if (blockWindow.getCurrentIndex() + windowSize < blockWindow.size()) {
                blockWindow.setCurrentIndex(blockWindow.size() - windowSize);
            }
        }
    }

private:
    double _sampleRate{44100};
    int _graphHeight{100};
    int _graphWidth{100};
    std::shared_ptr<State> _controls;
    std::vector<std::string> _scaleFactorStrings;
    std::vector<std::string> _millisecondStrings;

    using BlockWindow = common::SlidingWindow<common::audio::FrameBlock>;
    common::SingleReaderDirtyFlagged<BlockWindow> _blockWindow{BlockWindow{200}};
    ftxui::Canvas _canvas;
};

}
