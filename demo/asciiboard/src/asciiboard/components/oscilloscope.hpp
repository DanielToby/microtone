#pragma once

#include <ftxui/component/component.hpp>

#include <deque>

namespace asciiboard {

namespace detail {

constexpr auto scaleFactors = std::array{
    .25,
    .5,
    1.0,
    2.0,
    5.0
};

[[nodiscard]] double toMilliseconds(double numSamples, double sampleRate) {
    return numSamples / sampleRate * 1000;
}

}

class Oscilloscope {
public:
    Oscilloscope(double sampleRate, int graphWidth, int graphHeight, const std::shared_ptr<SynthControls>& controls) :
        _sampleRate(sampleRate),
        _graphHeight(graphHeight),
        _graphWidth(graphWidth) {
        using namespace ftxui;
        _component = Renderer([controls, this] {
            const auto getXValue = [this](std::size_t blockIndex, std::size_t i) {
                const auto totalSize = static_cast<double>(_numBlocksToShow * common::audio::AudioBlockSize);
                auto index = static_cast<double>(blockIndex * common::audio::AudioBlockSize + i);
                return static_cast<int>(index / totalSize * _graphWidth);
            };
            const auto getYValue = [this](float y) {
                const auto totalSize = 2.f; // [-1, 1].
                const auto value = y + 1; // Positive.

                return static_cast<int>(value / totalSize * detail::scaleFactors[_scaleFactorIndex] * _graphHeight);
            };
            auto c = Canvas(_graphWidth, _graphHeight);
            for (auto blockIndex = 0; blockIndex < _data.size(); ++blockIndex) {
                for (auto i = 0; i < common::audio::AudioBlockSize - 1; ++i) {
                    const auto x0 = getXValue(blockIndex, i);
                    const auto y0 = getYValue(_data.at(blockIndex).at(i));
                    const auto x1 = getXValue(blockIndex, i + 1);
                    const auto y1 = getYValue(_data.at(blockIndex).at(i + 1));
                    c.DrawPointLine(x0, y0, x1, y1, Color::Purple);
                }
            }

            return vbox({
                hbox({
                    text(fmt::format("{}x, ", detail::scaleFactors[_scaleFactorIndex])),
                    text(fmt::format(
                        "{:.2f}ms",
                        detail::toMilliseconds(static_cast<double>(_numBlocksToShow * common::audio::AudioBlockSize), _sampleRate))),
                }) | hcenter,

                vbox({
                    filler(),
                    canvas(std::move(c)) | hcenter,
                    filler(),
                }) | flex,
            });
        });
    }

    [[nodiscard]] const ftxui::Component& component() const {
        return _component;
    }
    void addAudioBlock(const common::audio::FrameBlock& block) {
        _data.push_back(block);
        if (_data.size() > _numBlocksToShow) {
            _data.pop_front();
        }
    }

private:
    double _sampleRate{44100};
    int _graphHeight{100};
    int _graphWidth{100};
    std::size_t _scaleFactorIndex{2};
    ftxui::Component _component;
    std::deque<common::audio::FrameBlock> _data;
    std::size_t _numBlocksToShow{50};
};

}
