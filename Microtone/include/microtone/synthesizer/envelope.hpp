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
    Envelope& operator=(const Envelope&) noexcept;
    Envelope(Envelope&&) noexcept;
    Envelope& operator=(Envelope&&) noexcept;
    ~Envelope();

    EnvelopeState state();
    double attack();
    double decay();
    double sustain();
    double release();

    void setAttack(double attack);
    void setDecay(double decay);
    void setSustain(double sustain);
    void setRelease(double release);

    void triggerOn();
    void triggerOff();
    float nextSample();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
