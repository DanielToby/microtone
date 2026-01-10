#pragma once

#include <synth/oscillator.hpp>
#include <synth/wave_table.hpp>

namespace synth {

class LowFrequencyOscillator {
public:
    LowFrequencyOscillator(double frequency, double sampleRate, float gain) :
        _oscillator{frequency, sampleRate},
        _weightedWaveTable{{buildWaveTable(examples::sineWaveFill)}, {gain}} {}

    LowFrequencyOscillator(const LowFrequencyOscillator& other) = default;

    float nextSample() {
        return _oscillator.nextSample(_weightedWaveTable);
    }

    void setFrequency(float frequencyHz) {
        _oscillator.setFrequency(frequencyHz);
    }

    void setGain(float gain) {
        _weightedWaveTable.weights[0] = gain;
    }

private:
    Oscillator _oscillator;
    WeightedWaveTables<1> _weightedWaveTable;
};

}
