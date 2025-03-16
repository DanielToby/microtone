#pragma once

#include <asciiboard/synth_controls.hpp>

#include <array>
#include <functional>
#include <memory>

#include <microtone/synthesizer/audio_buffer.hpp>

namespace asciiboard {

const int FRAMES_PER_BUFFER = 512;
using OnControlsChangedFn = std::function<void(const SynthControls&)>;

class Asciiboard {
public:
    Asciiboard();
    Asciiboard(const Asciiboard&) = delete;
    Asciiboard& operator=(const Asciiboard&) = delete;
    Asciiboard(Asciiboard&&) noexcept;
    Asciiboard& operator=(Asciiboard&&) noexcept;
    ~Asciiboard();

    void loop(const SynthControls& initialControls, const OnControlsChangedFn& onControlsChangedFn);
    void addOutputData(const microtone::AudioBuffer& data);
    void addMidiData(int status, int note, int velocity);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
