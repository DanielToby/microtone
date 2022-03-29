#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer_voice.hpp>
#include <microtone/oscillator.hpp>

#include <cmath>
#include <array>

namespace microtone {

const int WAVETABLE_LENGTH = 512;

class Oscillator::impl {
public:
    impl(WaveType waveType, double frequency, double sampleRate) :
        _waveType{waveType},
        _frequency{frequency},
        _sampleRate{sampleRate},
        _currentIndex{0} {
        fillTable();
    }

    impl(const impl& other) :
        _waveType{other._waveType},
        _frequency{other._frequency},
        _sampleRate{other._sampleRate},
        _currentIndex{0},
        _table{other._table} {
    }

    void fillTable() {
        for (auto i = 0; i < WAVETABLE_LENGTH; ++i) {
            _table[i] = std::sin(2.0 * M_PI * i / WAVETABLE_LENGTH);
        }
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

    WaveType _waveType;
    double _frequency;
    double _sampleRate;
    double _currentIndex;
    std::array<float, WAVETABLE_LENGTH> _table;
};

Oscillator::Oscillator(WaveType waveType, double frequency, double sampleRate) :
    _impl{new impl{waveType, frequency, sampleRate}} {
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
