#pragma once

#include <asciiboard/state.hpp>

#include <functional>
#include <memory>

#include <common/midi_handle.hpp>
#include <common/ring_buffer.hpp>

namespace asciiboard {

using OnControlsChangedFn = std::function<void(const State&)>;
using OnAboutToQuitFn = std::function<void()>;

class Asciiboard {
public:
    Asciiboard(const State& initialControls, double sampleRate);
    Asciiboard(const Asciiboard&) = delete;
    Asciiboard& operator=(const Asciiboard&) = delete;
    Asciiboard(Asciiboard&&) noexcept;
    Asciiboard& operator=(Asciiboard&&) noexcept;
    ~Asciiboard();

    void loop(const OnControlsChangedFn& onControlsChangedFn, const OnAboutToQuitFn& onAboutToQuitFn);
    void addOutputData(const common::audio::FrameBlock& lastAudioBlock);
    void updateMidiKeyboard(const common::midi::Keyboard& latestKeyboard);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
