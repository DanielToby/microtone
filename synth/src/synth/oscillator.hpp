#pragma once

#include <synth/wave_table.hpp>

#include <cmath>
#include <functional>
#include <memory>

namespace synth {

class Oscillator {
public:
    Oscillator(double frequency, double sampleRate) :
        _frequency{frequency},
        _sampleRate{sampleRate},
        _currentIndex{0} {}

    Oscillator(const Oscillator& other) :
        _frequency{other._frequency},
        _sampleRate{other._sampleRate},
        _currentIndex{0} {
    }

    template <std::size_t NumWaveTables>
    float nextSample(const WeightedWaveTables<NumWaveTables>& weightedWaveTables) {
        // Linear interpolation improves the signal approximation accuracy at discrete index.
        auto indexBelow = static_cast<int>(std::floor(_currentIndex));
        auto indexAbove = (indexBelow + 1) % WAVETABLE_LENGTH;
        auto fractionAbove = _currentIndex - indexBelow;
        auto fractionBelow = 1.0 - fractionAbove;
        _currentIndex = std::fmod((_currentIndex + WAVETABLE_LENGTH * _frequency / _sampleRate), WAVETABLE_LENGTH);

        auto nextSample = 0.0f;
        for (auto i = 0; i < NumWaveTables; i++) {
            const auto& waveTable = weightedWaveTables.waveTables[i];
            const auto& weight = weightedWaveTables.weights[i];
            nextSample += static_cast<float>((fractionBelow * waveTable[indexBelow] + fractionAbove * waveTable[indexAbove]) * weight);
        }

        return nextSample;
    }

    void setFrequency(float frequency) {
        _frequency = frequency;
    }

private:
    double _frequency;
    double _sampleRate;
    double _currentIndex;
};
}
