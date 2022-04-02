#include <microtone/synthesizer_voice.hpp>
#include <microtone/envelope.hpp>
#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/oscillator.hpp>

#include <cmath>

namespace microtone {

class SynthesizerVoice::impl {
public:
    impl(double frequency, Envelope envelope, Oscillator oscillator, Filter filter) :
        _frequency{frequency},
        _velocity(0),
        _envelope{envelope},
        _oscillator{oscillator},
        _filter{filter} {
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

    float nextSample() {
        return _envelope.nextSample() * _velocity * _oscillator.nextSample(); // _filter.nextSample(_envelope.nextSample() * _velocity * _oscillator.nextSample());
    }

    double _frequency;
    double _velocity;
    Envelope _envelope;
    Oscillator _oscillator;
    Filter _filter;
};

SynthesizerVoice::SynthesizerVoice(double frequency, Envelope envelope, Oscillator oscillator, Filter filter) :
    _impl{new impl{frequency, envelope, oscillator, filter}} {
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

float SynthesizerVoice::nextSample() {
    return _impl->nextSample();
}

}
