#pragma once

#include <synth/oscillator.hpp>
#include <synth/wave_table.hpp>

namespace synth {

class LowFrequencyOscillator {
public:
    LowFrequencyOscillator(double frequency, double sampleRate, double gain) :
        _oscillator{frequency, sampleRate},
        _weightedWaveTable{buildWaveTable(examples::sineWaveFill), gain} {}

    LowFrequencyOscillator(const LowFrequencyOscillator& other) = default;

    float nextSample() {
        return _oscillator.nextSample(_weightedWaveTable);
    }

    void setFrequency(float frequencyHz) {
        _oscillator.setFrequency(frequencyHz);
    }

    void setGain(double gain) {
        _weightedWaveTable.weight = gain;
    }

private:
    Oscillator _oscillator;
    WeightedWaveTable _weightedWaveTable;
};

}
