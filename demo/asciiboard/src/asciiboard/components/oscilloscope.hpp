#pragma once

#include <ftxui/component/component.hpp>

namespace asciiboard {

class Oscilloscope {
public:
    Oscilloscope(int graphHeight, double scaleFactor) :
        _graphHeight(graphHeight),
        _scaleFactor(scaleFactor) {
        using namespace ftxui;
        _component = Renderer([this] {
            const auto width = static_cast<int>(_latestAudioBlock.size());
            auto c = Canvas(width, _graphHeight);
            for (auto i = 0; i < width - 1; ++i) {
                c.DrawPointLine(i,
                                static_cast<int>(_latestAudioBlock[i] * _graphHeight * _scaleFactor) + (_graphHeight / 2),
                                i + 1,
                                static_cast<int>(_latestAudioBlock[i + 1] * _graphHeight * _scaleFactor) + (_graphHeight / 2),
                                Color::Purple);
            }

            return vbox({
                text(fmt::format("Frequency [Mhz] x ({})", _scaleFactor)) | hcenter,
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
    void setLatestAudioBlock(const common::audio::FrameBlock& block) {
        _latestAudioBlock = block;
    }

private:
    int _graphHeight = 100;
    double _scaleFactor = 1.0;
    ftxui::Component _component;
    common::audio::FrameBlock _latestAudioBlock{common::audio::emptyFrameBlock};
};

}
