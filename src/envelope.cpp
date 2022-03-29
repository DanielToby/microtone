#include <microtone/envelope.hpp>
#include <microtone/exception.hpp>
#include <microtone/log.hpp>

namespace microtone {

class Envelope::impl {
public:
    impl(double attack, double decay, double sustain, double release, double sampleRate) :
        _attack{attack},
        _decay{decay},
        _sustain{sustain},
        _release{release},
        _sampleRate{sampleRate},
        _currentValue{0},
        _increment{0},
        _counter{0},
        _state{EnvelopeState::Off} {}

    impl(const impl& other) :
        _attack{other._attack},
        _decay{other._decay},
        _sustain{other._sustain},
        _release{other._release},
        _sampleRate{other._sampleRate},
        _currentValue{other._currentValue},
        _increment{other._increment},
        _counter{other._counter},
        _state{other._state} {}

    EnvelopeState state() {
        return _state;
    }

    void triggerOn() {
        _state = EnvelopeState::Attack;
        rampTo(1.0, _attack); // perform attack
    }

    void triggerOff() {
        _state = EnvelopeState::Release;
        rampTo(0, _release); // perform release
    }

    void setValue(float value) {
        _currentValue = value;
        _increment = 0;
        _counter = 0;
    }

    void rampTo(float value, float time) {
        _increment = (value - _currentValue) / (_sampleRate * time);
        _counter = (int)(_sampleRate * time);
    }

    float nextSample() {
        if (_counter > 0) {
            _counter--;
            _currentValue += _increment;
        }

        if (_counter == 0) {
            if (_state == EnvelopeState::Attack) {
                _state = EnvelopeState::Decay;
                rampTo(_sustain, _decay); // perform decay
            } else if (_state == EnvelopeState::Decay) {
                _state = EnvelopeState::Sustain;
            }
            else if (_state == EnvelopeState::Release) {
                _state = EnvelopeState::Off;
            }
        }
        return _currentValue;
    }

    double _attack;
    double _decay;
    double _sustain;
    double _release;
    double _sampleRate;
    float _currentValue;
    float _increment;
    int _counter;
    EnvelopeState _state;
};

Envelope::Envelope(double attack, double decay, double sustain, double release, double sampleRate) :
    _impl{new impl{attack, decay, sustain, release, sampleRate}} {
}

Envelope::Envelope(const Envelope& other) :
    _impl{new impl{*other._impl}} {
}

Envelope::Envelope(Envelope&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Envelope& Envelope::operator=(Envelope&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

EnvelopeState Envelope::state() {
    return _impl->state();
}

Envelope::~Envelope() = default;

void Envelope::triggerOn() {
    _impl->triggerOn();
}

void Envelope::triggerOff() {
    _impl->triggerOff();
}

float Envelope::nextSample() {
    return _impl->nextSample();
}

}

