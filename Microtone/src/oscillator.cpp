#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/oscillator.hpp>

#include <cmath>
#include <array>

namespace microtone {

class Oscillator::impl {
public:
    impl(double frequency, double sampleRate) :
        _frequency{frequency},
        _sampleRate{sampleRate},
        _currentIndex{0} {}

    impl(const impl& other) :
        _frequency{other._frequency},
        _sampleRate{other._sampleRate},
        _currentIndex{0} {
    }

    float nextSample(const std::vector<WeightedWaveTable>& weightedWaveTables) {
        // Linear interpolation improves signal approximation accuracy at discrete index.
        auto indexBelow = static_cast<int>(floor(_currentIndex));
        auto indexAbove = indexBelow + 1;
        if (indexAbove >= WAVETABLE_LENGTH) {
            indexAbove = 0;
        }
        auto fractionAbove = _currentIndex - indexBelow;
        auto fractionBelow = 1.0 - fractionAbove;
        _currentIndex = std::fmod((_currentIndex + WAVETABLE_LENGTH * _frequency / _sampleRate), WAVETABLE_LENGTH);

        auto nextSample = 0.0;
        for (const auto& weightedWaveTable : weightedWaveTables) {
            nextSample += weightedWaveTable.weight * (fractionBelow * weightedWaveTable.waveTable[indexBelow]
                           + fractionAbove * weightedWaveTable.waveTable[indexAbove]);
        }

        return nextSample;
    }

    double _frequency;
    double _sampleRate;
    double _currentIndex;
};

Oscillator::Oscillator(double frequency, double sampleRate) :
    _impl{new impl{frequency, sampleRate}} {
}

Oscillator::Oscillator(const Oscillator& other) :
    _impl{new impl{*other._impl}} {
}

Oscillator::Oscillator(Oscillator&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Oscillator& Oscillator::operator=(const Oscillator& other) {
    _impl = std::make_unique<impl>(*other._impl);
    return *this;
}

Oscillator& Oscillator::operator=(Oscillator&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

Oscillator::~Oscillator() = default;

float Oscillator::nextSample(const std::vector<WeightedWaveTable>& weightedWaveTables) {
    return _impl->nextSample(weightedWaveTables);
}

}
