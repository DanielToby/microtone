#pragma once

#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/low_frequency_oscillator.hpp>
#include <synth/oscillator.hpp>
#include <synth/wave_table.hpp>

#include <memory>

namespace synth {

class SynthesizerVoice {
public:
    SynthesizerVoice(double frequency,
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

private:
    double _frequency;
    double _velocity;
    Envelope _envelope;
    Oscillator _oscillator;
    LowFrequencyOscillator _lfo;
    Filter _filter;
};

}
