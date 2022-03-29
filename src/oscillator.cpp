#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer_voice.hpp>
#include <microtone/oscillator.hpp>

#include <cmath>
#include <array>

namespace microtone {

class Oscillator::impl {
public:
    impl(double frequency, double sampleRate, WaveTableFn fillFn) :
        _frequency{frequency},
        _sampleRate{sampleRate},
        _currentIndex{0},
        _table{WAVETABLE_LENGTH} {

        fillFn(_table);
    }

    impl(const impl& other) :
        _frequency{other._frequency},
        _sampleRate{other._sampleRate},
        _currentIndex{0},
        _table{other._table} {
    }

    float nextSample() {
        // Linear interpolation improves signal approximation accuracy at discrete index.
        auto indexBelow = static_cast<int>(floor(_currentIndex));
        auto indexAbove = indexBelow + 1;
        if (indexAbove >= WAVETABLE_LENGTH) {
            indexAbove = 0;
        }
        auto fractionAbove = _currentIndex - indexBelow;
        auto fractionBelow = 1.0 - fractionAbove;

        _currentIndex = std::fmod((_currentIndex + WAVETABLE_LENGTH * _frequency / _sampleRate), WAVETABLE_LENGTH);

        return fractionBelow * _table[indexBelow] + fractionAbove * _table[indexAbove];
    }

    double _frequency;
    double _sampleRate;
    double _currentIndex;
    std::array<float, WAVETABLE_LENGTH> _table;
};

Oscillator::Oscillator(double frequency, double sampleRate, WaveTableFn fillFn) :
    _impl{new impl{frequency, sampleRate, fillFn}} {
}

Oscillator::Oscillator(const Oscillator& other) :
    _impl{new impl{*other._impl}} {
}

Oscillator::Oscillator(Oscillator&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Oscillator& Oscillator::operator=(Oscillator&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

Oscillator::~Oscillator() = default;

float Oscillator::nextSample() {
    return _impl->nextSample();
}

}
