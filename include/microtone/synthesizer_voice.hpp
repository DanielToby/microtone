#pragma once

#include <microtone/envelope.hpp>
#include <microtone/oscillator.hpp>

#include <cmath>

namespace microtone {

struct SynthesizerVoice {
    double frequency;
    int velocity;
    Envelope envelope;
    Oscillator oscillator;

    SynthesizerVoice() :
        frequency{-1},
        velocity{-1}
    {}

    SynthesizerVoice(double frequency, int velocity, Envelope envelope, Oscillator oscillator) :
        frequency{frequency},
        velocity{velocity},
        envelope{envelope},
        oscillator{oscillator} {
    }

    float nextSample() {
        const auto r = std::pow(10, 60 / 20);
        const auto b = 127 / (126 * sqrt(r)) - 1 / 126;
        const auto m = (1 - b) / 127;
        const auto v = std::pow(m * velocity + b, 2);

        const auto amplitude = envelope.nextSample();
        if (amplitude < 0.001) {
            return 0;
        }

        return amplitude * v * oscillator.nextSample(frequency);
    }
};

}
