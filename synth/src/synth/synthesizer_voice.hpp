#pragma once

#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/low_frequency_oscillator.hpp>
#include <synth/oscillator.hpp>
#include <synth/weighted_wavetable.hpp>

#include <memory>

namespace synth {

class SynthesizerVoice {
public:
    SynthesizerVoice(double frequency,
                              const Envelope& envelope,
                              const Oscillator& oscillator,
                              const LowFrequencyOscillator& lfo,
                              const Filter& filter);
    SynthesizerVoice(const SynthesizerVoice&) = delete;
    SynthesizerVoice& operator=(const SynthesizerVoice&) = delete;
    SynthesizerVoice(SynthesizerVoice&&) noexcept;
    SynthesizerVoice& operator=(SynthesizerVoice&&) noexcept;
    ~SynthesizerVoice();

    void setOscillator(const Oscillator& oscillator);
    void setEnvelope(const Envelope& envelope);
    void setFilter(const Filter& filter);

    bool isActive();
    void setVelocity(int velocity);
    void triggerOn();
    void triggerOff();
    float nextSample(const std::vector<WeightedWaveTable>& weightedWaveTables);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
