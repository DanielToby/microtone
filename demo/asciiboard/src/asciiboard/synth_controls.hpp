#pragma once

#include <synth/adsr.hpp>

namespace asciiboard {

struct SynthControls {
    int attack;
    int decay;
    int sustain;
    int release;

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
};

}
