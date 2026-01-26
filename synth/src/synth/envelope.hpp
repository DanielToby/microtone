#pragma once

#include <synth/adsr.hpp>

namespace synth {

enum class EnvelopeState {
    Attack = 0,
    Decay,
    Sustain,
    Release,
    Off
};

class Envelope {
public:
    Envelope(const ADSR& adsr, double sampleRate) :
        _adsr(adsr),
        _sampleRate{sampleRate},
        _currentValue{0},
        _increment{0},
        _counter{0},
        _state{EnvelopeState::Off} {}

    Envelope(const Envelope& other) = default;

    [[nodiscard]] EnvelopeState state() const {
        return _state;
    }

    [[nodiscard]] ADSR adsr() const { return _adsr; }

    void setAdsr(const ADSR& adsr) { _adsr = adsr; }

    void triggerOn() {
        _state = EnvelopeState::Attack;
        rampTo(1.0, _adsr.attack);
    }

    void triggerOff() {
        _state = EnvelopeState::Release;
        rampTo(0, _adsr.release);
    }

    void setValue(float value) {
        _currentValue = value;
        _increment = 0;
        _counter = 0;
    }

    void rampTo(double value, double time_s) {
        _increment = (value - _currentValue) / (_sampleRate * time_s);
        _counter = static_cast<int>(_sampleRate * time_s);
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
                rampTo(_adsr.sustain, _adsr.decay);
            } else if (_state == EnvelopeState::Decay) {
                _state = EnvelopeState::Sustain;
            } else if (_state == EnvelopeState::Release) {
                _state = EnvelopeState::Off;
            }
        }

        return _currentValue;
    }

private:
    ADSR _adsr;
    double _sampleRate;
    float _currentValue;
    double _increment;
    int _counter;
    EnvelopeState _state;
};

}
