#include <common/exception.hpp>
#include <synth/envelope.hpp>
#include <common/log.hpp>
#include <synth/synthesizer_voice.hpp>

#include <cmath>

namespace synth {

class SynthesizerVoice::impl {
public:
    impl(double frequency,
         const Envelope& envelope,
         const Oscillator& oscillator,
         const LowFrequencyOscillator& lfo,
         const Filter& filter) :
        _frequency{frequency},
        _velocity(0),
        _envelope{envelope},
        _oscillator{oscillator},
        _lfo{lfo},
        _filter{filter} {
    }

    void setOscillators(const Oscillator& oscillator) {
        _oscillator = oscillator;
    }

    void setEnvelope(const Envelope& envelope) {
        _envelope = envelope;
    }

    void setFilter(const Filter& filter) {
        _filter = filter;
    }

    bool isActive() {
        return _envelope.state() != EnvelopeState::Off;
    }

    void setVelocity(int velocity) {
        auto r = std::pow(10, 60 / 20);
        auto b = 127 / (126 * sqrt(r)) - 1 / 126;
        auto m = (1 - b) / 127;
        _velocity = std::pow(m * velocity + b, 2);
    }

    void triggerOn() {
        _envelope.triggerOn();
    }

    void triggerOff() {
        _envelope.triggerOff();
    }

    float nextSample(const std::vector<WeightedWaveTable>& weightedWaveTables) {
        auto nextSample = _oscillator.nextSample(weightedWaveTables);
        return _filter.nextSample(_envelope.nextSample() * static_cast<float>(_velocity) * nextSample);
    }

    double _frequency;
    double _velocity;
    Envelope _envelope;
    Oscillator _oscillator;
    LowFrequencyOscillator _lfo;
    Filter _filter;
};

SynthesizerVoice::SynthesizerVoice(double frequency,
                                   const Envelope& envelope,
                                   const Oscillator& oscillator,
                                   const LowFrequencyOscillator& lfo,
                                   const Filter& filter) :
    _impl{std::make_unique<impl>(frequency, envelope, oscillator, lfo, filter)} {
}

SynthesizerVoice::SynthesizerVoice(SynthesizerVoice&& other) noexcept :
    _impl{std::move(other._impl)} {
}

SynthesizerVoice& SynthesizerVoice::operator=(SynthesizerVoice&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

void SynthesizerVoice::setOscillator(const Oscillator& oscillator) {
    _impl->setOscillators(oscillator);
}

void SynthesizerVoice::setEnvelope(const Envelope& envelope) {
    _impl->setEnvelope(envelope);
}

void SynthesizerVoice::setFilter(const Filter& filter) {
    _impl->setFilter(filter);
}

SynthesizerVoice::~SynthesizerVoice() = default;

bool SynthesizerVoice::isActive() {
    return _impl->isActive();
}

void SynthesizerVoice::setVelocity(int velocity) {
    _impl->setVelocity(velocity);
}

void SynthesizerVoice::triggerOn() {
    _impl->triggerOn();
}

void SynthesizerVoice::triggerOff() {
    _impl->triggerOff();
}

float SynthesizerVoice::nextSample(const std::vector<WeightedWaveTable>& weightedWaveTables) {
    return _impl->nextSample(weightedWaveTables);
}

}
