#pragma once

#include <microtone/synthesizer_voice.hpp>

#include <memory>

namespace microtone {

enum class WaveType {
    Sine = 0,
    Square
};

class Oscillator {
public:
    explicit Oscillator(WaveType waveType, double frequency, double sampleRate);
    Oscillator(const Oscillator&);
    Oscillator& operator=(const Oscillator&) = delete;
    Oscillator(Oscillator&&) noexcept;
    Oscillator& operator=(Oscillator&&) noexcept;
    ~Oscillator();

    float nextSample();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
