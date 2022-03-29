#pragma once

#include <microtone/synthesizer_voice.hpp>

#include <functional>
#include <memory>

namespace microtone {

const int WAVETABLE_LENGTH = 512;

using WaveTable = std::array<float, WAVETABLE_LENGTH>;
using WaveTableFn = std::function<void(WaveTable& table)>;

class Oscillator {
public:
    explicit Oscillator(double frequency, double sampleRate, WaveTableFn fillFn);
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
