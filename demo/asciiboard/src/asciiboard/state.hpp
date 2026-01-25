#pragma once

#include <synth/effects/delay.hpp>
#include <synth/effects/modulated_filter.hpp>
#include <synth/synthesizer.hpp>

namespace asciiboard {

struct State {
    bool showInfoMessage{false};
    int selectedTab{0};

    bool isOscilloscopeLive{false};
    int oscilloscopeScaleFactorIndex{0};
    int oscilloscopeTimelineSizeIndex{0};

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

    int filterTypeIndex{0};
    float filterCutoffFrequencyHz{0.f};
    float filterLfoDepthHz{0.f};
    float filterLfoFrequencyHz{0.f};

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

    //! Applies any updated controls relevant to the synthesizer.
    void applyChanges(synth::Synthesizer& synth, const State& newControls) const {
        if (this->getOscillatorWeights() != newControls.getOscillatorWeights()) {
            synth.setOscillatorWeights(newControls.getOscillatorWeights());
        }

        if (this->gain != newControls.gain) {
            synth.setGain(newControls.gain);
        }

        if (this->lfoFrequency_Hz != newControls.lfoFrequency_Hz) {
            synth.setLfoFrequency(newControls.lfoFrequency_Hz);
        }

        if (this->lfoGain != newControls.lfoGain) {
            synth.setLfoGain(newControls.lfoGain);
        }

        if (this->getAdsr() != newControls.getAdsr()) {
            synth.setAdsr(newControls.getAdsr());
        }
    }

    //! Applies any updated controls relevant to the Delay effect.
    void applyChanges(synth::Delay& delay, const State& newControls, double sampleRate) const {
        if (this->delay_ms != newControls.delay_ms) {
            delay.setDelay(newControls.getDelay_samples(sampleRate));
        }

        if (this->delayGain != newControls.delayGain) {
            delay.setGain(newControls.delayGain);
        }
    }

    //! Applies any updated controls relevant to the modulated filter.
    void applyChanges(synth::ModulatedFilter& filter, const State& newControls) const {
        if (this->filterTypeIndex != newControls.filterTypeIndex) {
            filter.setFilterType(static_cast<synth::FilterType>(newControls.filterTypeIndex));
        }
        if (this->filterCutoffFrequencyHz != newControls.filterCutoffFrequencyHz) {
            filter.setCutoffFrequencyHz(newControls.filterCutoffFrequencyHz);
        }
        if (this->filterLfoDepthHz != newControls.filterLfoDepthHz) {
            filter.setLFODepthHz(newControls.filterLfoDepthHz);
        }
        if (this->filterLfoFrequencyHz != newControls.filterLfoFrequencyHz) {
            filter.setLFOFrequencyHz(newControls.filterLfoFrequencyHz);
        }
    }
};

}
