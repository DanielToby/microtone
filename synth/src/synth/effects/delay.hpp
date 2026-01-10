#pragma once

#include "synth/audio_pipeline.hpp"

namespace synth {

//! Records a history of the samples that pass through this class, then feeds them into the input.
class Delay : public I_FunctionNode {
public:
    Delay(std::size_t numSamples, float gain) :
        _state(State{numSamples, gain}) {
        throwIfInvalid();
    }

    [[nodiscard]] float transform(float in) override {
        auto out = in;
        _state.write([&out, this](State& state) {
            out += state.gain * state.memory[state.tail];

            state.memory[state.head] = out;
            state.head = (state.head + 1) % state.memory.size();
            state.tail = (state.tail + 1) % state.memory.size();
        });
        return out;
    }

    void setDelay(std::size_t numSamples) {
        _state.write([&numSamples](State& state) {
            state.head = numSamples;
            state.tail = 0;
        });
    }

    void setGain(float gain) {
        _state.write([&gain](State& state) {
            state.gain = gain;
        });
    }

private:
    void throwIfInvalid() const {
        const auto& state = _state.read();
        const auto delay = state.head - state.tail;
        if (delay == 0 || delay >= state.memory.size()) {
            throw common::MicrotoneException("Delay duration is outside of allowable bounds.");
        }
    }

    struct State {
        State(std::size_t numSamples, float gain) : head{numSamples}, gain{gain} {}

        // This class supports a delay of up to 1s at a sample rate of 48 kHz.
        std::array<float, 48000> memory{0.f};
        std::size_t head{0};
        std::size_t tail{0};

        float gain{1.f};
    };

    common::MutexProtected<State> _state;
};

}