#include <microtone/synthesizer_voice.hpp>
#include <microtone/envelope.hpp>
#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/oscillator.hpp>

namespace microtone {

class SynthesizerVoice::impl {
public:
    impl(double frequency, Envelope envelope, Oscillator oscillator) :
        _frequency{frequency},
        _velocity(0),
        _envelope{envelope},
        _oscillator{oscillator} {
    }

    bool isActive() {
        return _envelope.state() != EnvelopeState::Off;
    }

    void setVelocity(int velocity) {
        _velocity = velocity;
    }

    void triggerOn() {
        _envelope.triggerOn();
    }

    void triggerOff() {
        _envelope.triggerOff();
    }

    float nextSample() {
        const auto r = std::pow(10, 60 / 20);
        const auto b = 127 / (126 * sqrt(r)) - 1 / 126;
        const auto m = (1 - b) / 127;
        const auto v = std::pow(m * _velocity + b, 2);
        return _envelope.nextSample() * v * _oscillator.nextSample();
    }

    double _frequency;
    int _velocity;
    Envelope _envelope;
    Oscillator _oscillator;
};

SynthesizerVoice::SynthesizerVoice(double frequency, Envelope envelope, Oscillator oscillator) :
    _impl{new impl{frequency, envelope, oscillator}} {
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
