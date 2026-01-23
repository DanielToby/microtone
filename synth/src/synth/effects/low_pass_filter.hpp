#pragma once

#include "synth/audio_pipeline.hpp"
#include "synth/math.hpp"

namespace synth {

//! Implements an analog low-pass filter, discretized using the forward Euler method.
class LowPassFilter : public I_FunctionNode {
public:
    LowPassFilter(double sampleRate, float cutoffFrequencyHz) :
        _state(State{sampleRate, cutoffFrequencyHz}) {}

    [[nodiscard]] float transform(float in) override {
        auto out = in;
        _state.write([&](State& state) {
            out = state.alpha * in + state.beta * state.lastOutput;
            state.lastOutput = out;
        });
        return out;
    }

    void setCutoffFrequencyHz(float frequencyHz) {
        _state.write([&frequencyHz](State& state) {
            state.alpha = computeAlpha(state.sampleRate, frequencyHz);
            state.beta = computeBeta(state.sampleRate, frequencyHz);
        });
    }

private:
    [[nodiscard]] static float computeAlpha(double sampleRate, float cutoffFrequencyHz) {
        const auto T = 1 / sampleRate;
        const auto RC = 1 / (2 * M_PI * cutoffFrequencyHz);
        return T / (T + RC);
    }

    [[nodiscard]] static float computeBeta(double sampleRate, float cutoffFrequencyHz) {
        const auto T = 1 / sampleRate;
        const auto RC = 1 / (2 * M_PI * cutoffFrequencyHz);
        return RC / (T + RC);
    }

    struct State {
        State(double sampleRate, float cutoffFrequencyHz) :
            sampleRate{sampleRate},
            alpha{computeAlpha(sampleRate, cutoffFrequencyHz)},
            beta{computeBeta(sampleRate, cutoffFrequencyHz)} {}

        float lastOutput{0};

        double sampleRate{44100.0};

        // Precomputed for performance.
        float alpha;
        float beta;
    };

    common::MutexProtected<State> _state;
};

}