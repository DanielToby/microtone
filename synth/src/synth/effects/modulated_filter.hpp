#pragma once

#include "synth/audio_pipeline.hpp"
#include "synth/effects/high_pass_filter.hpp"
#include "synth/effects/low_pass_filter.hpp"
#include "synth/low_frequency_oscillator.hpp"

#include <variant>

namespace synth {

enum class FilterType {
    LowPass = 0,
    HighPass
};

//! A filter whose cutoff frequency is modulated by a low frequency oscillator.
class ModulatedFilter : public I_FunctionNode {
public:
    ModulatedFilter(double sampleRate, FilterType type, float cutoffFrequencyHz, float lfoDepthHz, float lfoFrequencyHz) :
        _state(State{
            makeFilter(type, sampleRate, cutoffFrequencyHz),
            LowFrequencyOscillator(lfoFrequencyHz, sampleRate, 1.0),
            sampleRate,
            cutoffFrequencyHz,
            lfoDepthHz}) {}

    [[nodiscard]] float transform(float in) override {
        auto out = in;
        _state.write([&out](State& state) {
            applyLFOToFilterCutoff(state.lfo, state.filter, state.cutoffFrequencyHz, state.lfoDepthHz);
            out = std::visit([&out](auto& f) { return f.transform(out); }, state.filter);
        });
        return out;
    }

    void setFilterType(FilterType type) {
        _state.write([&](State& state) { state.filter = makeFilter(type, state.sampleRate, state.cutoffFrequencyHz); });
    }

    void setCutoffFrequencyHz(float frequencyHz) {
        _state.write([&](State& state) { state.cutoffFrequencyHz = frequencyHz; });
    }

    void setLFODepthHz(float depthHz) {
        _state.write([&](State& state) { state.lfoDepthHz = depthHz; });
    }

    void setLFOFrequencyHz(float frequencyHz) {
        _state.write([&](State& state) {
            state.lfo.setFrequency(frequencyHz);
        });
    }

private:
    using FilterT = std::variant<LowPassFilter, HighPassFilter>;

    [[nodiscard]] static FilterT makeFilter(FilterType type, double sampleRate, float cutoffFrequencyHz) {
        switch (type) {
        case FilterType::LowPass:
            return LowPassFilter(sampleRate, cutoffFrequencyHz);
        case FilterType::HighPass:
            return HighPassFilter(sampleRate, cutoffFrequencyHz);
        default:
            throw common::MicrotoneException("Unsupported filter type.");
        }
    }

    //! The core function of this class: sweeping the filter cutoff up and down using the LFO.
    static void applyLFOToFilterCutoff(LowFrequencyOscillator& lfo, FilterT& filterVariant, float baseFrequencyHz, float frequencyDepthHz) {
        auto lfoOutput = lfo.nextSample();
        auto newCutoff = baseFrequencyHz + lfoOutput * frequencyDepthHz;
        std::visit([&newCutoff](auto& f) { f.setCutoffFrequencyHz(newCutoff); }, filterVariant);
    }

    struct State {
        FilterT filter;
        LowFrequencyOscillator lfo;

        double sampleRate;
        float cutoffFrequencyHz;
        float lfoDepthHz;
    };

    common::MutexProtected<State> _state;
};

}