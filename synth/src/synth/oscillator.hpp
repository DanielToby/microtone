#pragma once

#include <synth/wave_table.hpp>

#include <cmath>
#include <functional>
#include <memory>

namespace synth {

class Oscillator {
public:
    Oscillator(double frequency, double sampleRate) :
        _increment{toIncrement(frequency, sampleRate)},
        _sampleRate{sampleRate},
        _currentIndex{0} {}

    Oscillator(const Oscillator& other) :
        _increment{other._increment},
        _sampleRate{other._sampleRate},
        _currentIndex{0} {
    }

    template <std::size_t NumWaveTables>
    float nextSample(const WeightedWaveTables<NumWaveTables>& weightedWaveTables) {
        // Linear interpolation improves the signal approximation accuracy at discrete indices.
        auto indexBelow = static_cast<int>(std::floor(_currentIndex));
        auto indexAbove = (indexBelow + 1) % WAVETABLE_LENGTH;
        auto fractionAbove = _currentIndex - indexBelow;
        auto fractionBelow = 1 - fractionAbove;
        _currentIndex = std::fmod(_currentIndex + _increment, WAVETABLE_LENGTH);

        auto nextSample = 0.0f;
        for (auto i = 0; i < NumWaveTables; i++) {
            const auto& waveTable = weightedWaveTables.waveTables[i];
            const auto& weight = weightedWaveTables.weights[i];
            nextSample += static_cast<float>((fractionBelow * waveTable[indexBelow] + fractionAbove * waveTable[indexAbove]) * weight);
        }

        return nextSample;
    }

    void setFrequency(double frequency) {
        _increment = toIncrement(frequency, _sampleRate);
    }

private:
    [[nodiscard]] static double toIncrement(double frequency, double sampleRate) {
        return WAVETABLE_LENGTH * frequency / sampleRate;
    }

    double _sampleRate;
    double _increment;
    double _currentIndex;
};
}
