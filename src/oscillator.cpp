#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer_voice.hpp>
#include <microtone/oscillator.hpp>

namespace microtone {

class Oscillator::impl {
public:
    impl(WaveType waveType, double sampleRate, double lfoFrequency, double lfoAmplitude) :
        _waveType{waveType},
        _sampleRate{sampleRate},
        _phase{0},
        _lfoPhase{0},
        _lfoFrequency{lfoFrequency},
        _lfoAmplitude{lfoAmplitude} {}

    impl(const impl& other) :
        _waveType{other._waveType},
        _sampleRate{other._sampleRate},
        _phase{other._phase},
        _lfoPhase{other._lfoPhase},
        _lfoFrequency{other._lfoFrequency},
        _lfoAmplitude{other._lfoAmplitude} {}

    float nextSample(double frequency) {
        auto delta = 2.0f * frequency * static_cast<float>(M_PI / _sampleRate);
        _phase = std::fmod(_phase + delta, 2.0f * float(M_PI));

        auto lfoDelta = 2.0f * _lfoFrequency * static_cast<float>(M_PI / _sampleRate);
        _lfoPhase = std::fmod(_lfoPhase + lfoDelta, 2.0f * float(M_PI));
        auto lfo = _lfoAmplitude * std::sin(_lfoPhase);

        switch (_waveType) {
        case WaveType::Sine:
            return std::sin(_phase + lfo);
        case WaveType::Square:
            return std::copysign(0.1f, std::sin(_phase + lfo));
        }
    }

    WaveType _waveType;
    double _sampleRate;
    double _phase;
    double _lfoPhase;
    double _lfoFrequency;
    double _lfoAmplitude;
};

Oscillator::Oscillator(WaveType waveType, double sampleRate, double lfoFrequency, double lfoAmplitude) :
    _impl{new impl{waveType, sampleRate, lfoFrequency, lfoAmplitude}} {
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

float Oscillator::nextSample(double frequency) {
    return _impl->nextSample(frequency);
}

}
