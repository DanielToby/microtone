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
    explicit Oscillator(WaveType waveType = WaveType::Sine,
                        double sampleRate = -1,
                        double lfoFrequency = 2,
                        double lfoAmplitude = 1.0);
    Oscillator(const Oscillator&);
    Oscillator& operator=(const Oscillator&) = delete;
    Oscillator(Oscillator&&) noexcept;
    Oscillator& operator=(Oscillator&&) noexcept;
    ~Oscillator();

    float nextSample(double frequency);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
