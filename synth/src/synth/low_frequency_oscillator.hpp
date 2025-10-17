#pragma once

#include <synth/oscillator.hpp>
#include <synth/wave_table.hpp>

namespace synth {

// This class is a wrapper around Oscillator that manages a sinusoidal wavetable.

class LowFrequencyOscillator {
public:
    LowFrequencyOscillator(double frequency, double sampleRate) :
        _oscillator{frequency, sampleRate},
        _waveTable{} {
        for (auto i = 0; i < synth::WAVETABLE_LENGTH; ++i) {
            _waveTable[i] = static_cast<float>(std::sin(2.0 * M_PI * i / synth::WAVETABLE_LENGTH));
        }
    }

    LowFrequencyOscillator(const LowFrequencyOscillator& other) = default;

    float nextSample() {
        return _oscillator.nextSample({{_waveTable, 1.0}});
    }

private:
    Oscillator _oscillator;
    WaveTable _waveTable;
};

}
