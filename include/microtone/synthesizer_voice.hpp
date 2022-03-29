#pragma once

#include <microtone/envelope.hpp>
#include <microtone/oscillator.hpp>

#include <memory>

namespace microtone {

class SynthesizerVoice {
public:
    explicit SynthesizerVoice(double frequency, Envelope envelope, Oscillator oscillator);
    SynthesizerVoice(const SynthesizerVoice&) = delete;
    SynthesizerVoice& operator=(const SynthesizerVoice&) = delete;
    SynthesizerVoice(SynthesizerVoice&&) noexcept;
    SynthesizerVoice& operator=(SynthesizerVoice&&) noexcept;
    ~SynthesizerVoice();

    bool isActive();
    void setVelocity(int velocity);
    void triggerOn();
    void triggerOff();
    float nextSample();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
