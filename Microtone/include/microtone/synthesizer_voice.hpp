#pragma once

#include <microtone/microtone_platform.hpp>

#include <microtone/envelope.hpp>
#include <microtone/filter.hpp>
#include <microtone/oscillator.hpp>
#include <microtone/weighted_wavetable.hpp>

#include <memory>

namespace microtone {

class SynthesizerVoice {
public:
    explicit SynthesizerVoice(double frequency,
                              const Oscillator& oscillator,
                              const Envelope& envelope,
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
