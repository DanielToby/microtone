#pragma once

#include <synth/weighted_wavetable.hpp>

#include <functional>
#include <memory>

namespace synth {

class Oscillator {
public:
    Oscillator(double frequency, double sampleRate);
    Oscillator(const Oscillator&);
    Oscillator& operator=(const Oscillator&);
    Oscillator(Oscillator&&) noexcept;
    Oscillator& operator=(Oscillator&&) noexcept;
    ~Oscillator();

    float nextSample(const std::vector<WeightedWaveTable>& weightedWaveTables);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
