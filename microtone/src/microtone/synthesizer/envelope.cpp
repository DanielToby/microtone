#include <microtone/exception.hpp>
#include <microtone/synthesizer/envelope.hpp>
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

    double attack() {
        return _attack;
    }

    double decay() {
        return _decay;
    }

    double sustain() {
        return _sustain;
    }

    double release() {
        return _release;
    }

    void setAttack(double attack) {
        _attack = attack;
    }

    void setDecay(double decay) {
        _decay = decay;
    }

    void setSustain(double sustain) {
        _sustain = sustain;
    }

    void setRelease(double release) {
        _release = release;
    }

    void triggerOn() {
        _state = EnvelopeState::Attack;
        rampTo(1.0, _attack);                                   // perform attack
    }

    void triggerOff() {
        _state = EnvelopeState::Release;
        rampTo(0, _release);                                    // perform release
    }

    void setValue(float value) {
        _currentValue = value;
        _increment = 0;
        _counter = 0;
    }

    void rampTo(double value, double time) {
        _increment = (value - _currentValue) / (_sampleRate * time);
        _counter = static_cast<int>(_sampleRate * time);
    }

    float nextSample() {
        if (_counter > 0) {
            _counter--;
            _currentValue += static_cast<float>(_increment);
        }

        // Finished current phase.
        if (_counter == 0) {
            if (_state == EnvelopeState::Attack) {
                _state = EnvelopeState::Decay;
                rampTo(_sustain, _decay);                       // perform decay
            } else if (_state == EnvelopeState::Decay) {
                _state = EnvelopeState::Sustain;
            } else if (_state == EnvelopeState::Release) {
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
    double _increment;
    int _counter;
    EnvelopeState _state;
};

Envelope::Envelope(double attack, double decay, double sustain, double release, double sampleRate) :
    _impl{std::make_unique<impl>(attack, decay, sustain, release, sampleRate)} {
}

Envelope::Envelope(const Envelope& other) :
    _impl{std::make_unique<impl>(*other._impl)} {
}

Envelope::Envelope(Envelope&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Envelope& Envelope::operator=(const Envelope& other) noexcept {
    _impl = std::make_unique<impl>(*other._impl);
    return *this;
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

double Envelope::attack() {
    return _impl->attack();
}

double Envelope::decay() {
    return _impl->decay();
}

double Envelope::sustain() {
    return _impl->sustain();
}

double Envelope::release() {
    return _impl->release();
}

void Envelope::setAttack(double attack) {
    _impl->setAttack(attack);
}

void Envelope::setDecay(double decay) {
    _impl->setDecay(decay);
}

void Envelope::setSustain(double sustain) {
    _impl->setSustain(sustain);
}

void Envelope::setRelease(double release) {
    _impl->setRelease(release);
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
