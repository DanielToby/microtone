#pragma once

#include "synth/audio_pipeline.hpp"
#include "synth/math.hpp"

namespace synth {

//! Implements an analog high-pass filter, discretized using the forward Euler method.
//! Note: High cutoffs perform poorly with the forward Euler method. The Tustin method is apparently better (less shallow)
class HighPassFilter : public I_FunctionNode {
public:
    HighPassFilter(double sampleRate, float cutoffFrequencyHz) :
        _state(State{sampleRate, cutoffFrequencyHz}) {}

    HighPassFilter(const HighPassFilter& other) : _state(other._state) {}
    HighPassFilter& operator=(const HighPassFilter& other) {
        _state = other._state;
        return *this;
    }

    [[nodiscard]] float transform(float in) override {
        auto out = in;
        _state.write([&](State& state) {
            out = state.alpha * (in  - state.lastInput + state.lastOutput);
            state.lastInput = in;
            state.lastOutput = out;
        });
        return out;
    }

    void setCutoffFrequencyHz(float frequencyHz) {
        _state.write([&frequencyHz](State& state) {
            state.alpha = computeAlpha(state.sampleRate, frequencyHz);
        });
    }

private:
    [[nodiscard]] static float computeAlpha(double sampleRate, float cutoffFrequencyHz) {
        const auto T = 1 / sampleRate;
        const auto RC = 1 / (2 * M_PI * cutoffFrequencyHz);
        return RC / (T + RC);
    }

    struct State {
        State(double sampleRate, float cutoffFrequencyHz) :
            sampleRate{sampleRate},
            alpha{computeAlpha(sampleRate, cutoffFrequencyHz)} {}

        float lastInput{0};
        float lastOutput{0};

        double sampleRate{44100.0};

        // Precomputed for performance.
        float alpha;
    };

    common::MutexProtected<State> _state;
};

}