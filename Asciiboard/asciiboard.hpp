#pragma once

#include <array>
#include <functional>
#include <memory>

#include <microtone/synthesizer/audio_buffer.hpp>

namespace asciiboard {

const int FRAMES_PER_BUFFER = 512;
using OnEnvelopeChangedFn = std::function<void(double attack, double decay, double sustain, double release)>;
using OnFilterChangedFn = std::function<void()>;

class Asciiboard {
public:
    explicit Asciiboard();
    Asciiboard(const Asciiboard&) = delete;
    Asciiboard& operator=(const Asciiboard&) = delete;
    Asciiboard(Asciiboard&&) noexcept;
    Asciiboard& operator=(Asciiboard&&) noexcept;
    ~Asciiboard();

    void addOutputData(const microtone::AudioBuffer& data);
    void addMidiData(int status, int note, int velocity);
    void loop(const OnEnvelopeChangedFn& onEnvelopeChangedFn);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};


}
