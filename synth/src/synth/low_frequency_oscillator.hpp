#pragma once

#include <memory>

namespace synth {

// This class is a wrapper around Oscillator that manages a sinusoidal wavetable.

class LowFrequencyOscillator {
public:
    LowFrequencyOscillator(double frequency = 0, double sampleRate = -1);
    LowFrequencyOscillator(const LowFrequencyOscillator&);
    LowFrequencyOscillator(LowFrequencyOscillator&&) noexcept;
    LowFrequencyOscillator& operator=(const LowFrequencyOscillator&) noexcept;
    LowFrequencyOscillator& operator=(LowFrequencyOscillator&&) noexcept;
    ~LowFrequencyOscillator();

    float nextSample();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
