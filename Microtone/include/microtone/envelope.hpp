#pragma once

#include <microtone/microtone_platform.hpp>

#include <memory>

namespace microtone {

enum class EnvelopeState {
    Attack = 0,
    Decay,
    Sustain,
    Release,
    Off
};

class Envelope {
public:
    explicit Envelope(double attack, double decay, double sustain, double release, double sampleRate);
    Envelope(const Envelope&);
    Envelope& operator=(const Envelope&) = delete;
    Envelope(Envelope&&) noexcept;
    Envelope& operator=(Envelope&&) noexcept;
    ~Envelope();

    EnvelopeState state();
    void triggerOn();
    void triggerOff();
    float nextSample();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
