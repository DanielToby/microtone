#pragma once

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
    explicit Envelope(double attack = 0, double decay = 0, double sustain = 1.0, double release = 0, double sampleRate = 1);
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
