#pragma once

#include <microtone/microtone_platform.hpp>

#include <microtone/synthesizer/envelope.hpp>
#include <microtone/synthesizer/filter.hpp>
#include <microtone/synthesizer/low_frequency_oscillator.hpp>
#include <microtone/synthesizer/oscillator.hpp>
#include <microtone/synthesizer/weighted_wavetable.hpp>

#include <memory>

namespace microtone {

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
