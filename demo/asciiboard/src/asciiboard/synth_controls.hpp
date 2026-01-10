#pragma once

#include <synth/adsr.hpp>

#include <cstdio>

namespace asciiboard {

struct SynthControls {
    int attack{0};
    int decay{0};
    int sustain{0};
    int release{0};

    int sineWeight{0};
    int squareWeight{0};
    int triangleWeight{0};

    float gain{0.f};
    float lfoFrequencyHz{0.f};
    float lfoGain{0.f};

    float delay_ms{0.f};
    float delayGain{0.f};

    [[nodiscard]] synth::ADSR getAdsr() const {
        return {attack / 100., decay / 100., sustain / 100., release / 100.};
    }

    [[nodiscard]] std::size_t getDelay_samples(double sampleRate) const {
        return static_cast<std::size_t>(delay_ms / 1000 * sampleRate);
    }
};

}
