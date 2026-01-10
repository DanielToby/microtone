#pragma once

#include <synth/synthesizer.hpp>

namespace asciiboard {

struct SynthControls {
    int attack_pct{0};
    int decay_pct{0};
    int sustain_pct{0};
    int release_pct{0};

    int sineWeight_pct{0};
    int squareWeight_pct{0};
    int triangleWeight_pct{0};

    float gain{0.f};
    float lfoFrequency_Hz{0.f};
    float lfoGain{0.f};

    float delay_ms{0.f};
    float delayGain{0.f};

    [[nodiscard]] synth::TripleWeightsT getOscillatorWeights() const {
        return {
            static_cast<float>(sineWeight_pct / 100.),
            static_cast<float>(squareWeight_pct / 100.),
            static_cast<float>(triangleWeight_pct / 100.),
        };
    }

    [[nodiscard]] synth::ADSR getAdsr() const {
        return {attack_pct / 100., decay_pct / 100., sustain_pct / 100., release_pct / 100.};
    }

    [[nodiscard]] std::size_t getDelay_samples(double sampleRate) const {
        return static_cast<std::size_t>(delay_ms / 1000 * sampleRate);
    }
};

}
