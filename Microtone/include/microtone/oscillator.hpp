#pragma once

#include <microtone/microtone_platform.hpp>
#include <microtone/weighted_wavetable.hpp>

#include <functional>
#include <memory>

namespace microtone {

class Oscillator {
public:
    explicit Oscillator(double frequency, double sampleRate);
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
