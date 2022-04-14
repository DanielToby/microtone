#include <microtone/synthesizer/low_frequency_oscillator.hpp>
#include <microtone/synthesizer/oscillator.hpp>
#include <microtone/synthesizer/wavetable.hpp>

#include <cmath>
#include <vector>

namespace microtone {

class LowFrequencyOscillator::impl {
public:
    impl(double frequency, double sampleRate) :
        _oscillator{frequency, sampleRate},
        _waveTable{} {
        for (auto i = 0; i < microtone::WAVETABLE_LENGTH; ++i) {
            _waveTable[i] = std::sin(2.0 * M_PI * i / microtone::WAVETABLE_LENGTH);
        }
    }

    impl(const impl& other) :
        _oscillator{other._oscillator},
        _waveTable{other._waveTable} {}

    float nextSample() {
        return _oscillator.nextSample({{_waveTable, 1.0}});
    }

    Oscillator _oscillator;
    WaveTable _waveTable;
};

LowFrequencyOscillator::LowFrequencyOscillator(double frequency, double sampleRate) :
    _impl{new impl{frequency, sampleRate}} {
}

LowFrequencyOscillator::LowFrequencyOscillator(const LowFrequencyOscillator& other) :
    _impl{new impl{*other._impl}} {
}

LowFrequencyOscillator::LowFrequencyOscillator(LowFrequencyOscillator&& other) noexcept :
    _impl{std::move(other._impl)} {
}


LowFrequencyOscillator& LowFrequencyOscillator::operator=(const LowFrequencyOscillator& other) noexcept {
    _impl = std::make_unique<impl>(*other._impl);
    return *this;
}

LowFrequencyOscillator& LowFrequencyOscillator::operator=(LowFrequencyOscillator&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

float LowFrequencyOscillator::nextSample() {
    return _impl->nextSample();
}

LowFrequencyOscillator::~LowFrequencyOscillator() = default;

}
