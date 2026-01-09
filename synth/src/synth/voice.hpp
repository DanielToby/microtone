#pragma once

#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/low_frequency_oscillator.hpp>
#include <synth/oscillator.hpp>
#include <synth/wave_table.hpp>

namespace synth {

class Voice {
public:
    Voice(double frequency,
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

    void setLfoFrequency(float frequencyHz) {
        _lfo.setFrequency(frequencyHz);
    }

    bool isActive() {
        return _envelope.state() != EnvelopeState::Off;
    }

    //! This expects a midi-like velocity.
    void triggerOn(int velocity) {
        this->setVelocity(velocity);
        _envelope.triggerOn();
    }

    void triggerOff() {
        _envelope.triggerOff();
    }

    float nextSample(const std::vector<WeightedWaveTable>& weightedWaveTables) {
        auto velocityScalar = static_cast<float>(_velocity);
        auto nextOscillatorOutput = _oscillator.nextSample(weightedWaveTables);
        auto envelopeScalar = _envelope.nextSample();

        if (!this->isActive()) {
            this->setVelocity(0);
        }

        return _filter.nextSample(nextOscillatorOutput * velocityScalar * envelopeScalar);
    }

private:
    void setVelocity(int velocity) {
        auto r = std::pow(10, 60 / 20);
        auto b = 127 / (126 * sqrt(r)) - 1 / 126;
        auto m = (1 - b) / 127;
        _velocity = std::pow(m * velocity + b, 2);
    }

    double _frequency;
    double _velocity;
    Envelope _envelope;
    Oscillator _oscillator;
    LowFrequencyOscillator _lfo;
    Filter _filter;
};

}
