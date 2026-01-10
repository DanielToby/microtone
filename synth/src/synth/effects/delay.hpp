#pragma once

#include "synth/audio_pipeline.hpp"

namespace synth {

//! Records a history of the samples that pass through this class, then feeds them into the input.
class Delay : public I_FunctionNode {
public:
    Delay(std::size_t numSamples, float gain) : _head(numSamples), _gain(gain) {
        if (numSamples == 0 || numSamples > _memory.size()) {
            throw common::MicrotoneException("Delay duration is outside of allowable bounds.");
        }
    }

    [[nodiscard]] float transform(float in) override {
        // It's safe to apply this even when we haven't seen numSamples yet, because the buffer is zero-initialized.
        auto out = in + _gain * _memory[_tail];

        _memory[_head] = out;
        _head = (_head + 1) % _memory.size();
        _tail = (_tail + 1) % _memory.size();

        return out;
    }

private:
    float _gain;

    // This class supports a delay of up to 1s at a sample rate of 48 kHz.
    std::array<float, 48000> _memory{0.f};
    std::size_t _head; // Initialized to _numSamples.
    std::size_t _tail{0};
};

}