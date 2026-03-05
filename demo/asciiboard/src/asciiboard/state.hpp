#pragma once

#include <synth/effects/delay.hpp>
#include <synth/effects/modulated_filter.hpp>
#include <synth/synthesizer.hpp>

namespace asciiboard {

struct I_Incrementable {
    virtual ~I_Incrementable() = default;
    virtual void increment() = 0;
    virtual void decrement() = 0;
};

template <typename T>
struct Numeric : I_Incrementable {
    T value;
    T min;
    T max;
    T incrementAmount;

    Numeric(T initial, T min, T max) : value{initial}, min{min}, max{max}, incrementAmount{fromPercent(1, min, max)} {
        if (initial < min || initial > max) {
            throw std::out_of_range("initial must be between min and max.");
        }
    }

    [[nodiscard]] bool operator==(const Numeric& other) const {
        return value == other.value;
    }

    [[nodiscard]] bool operator!=(const Numeric& other) const {
        return !(*this == other);
    }

    [[nodiscard]] static int toPercent(T value, T min, T max) {
        auto fractionAlong = (value - min) / (max - min);
        return fractionAlong * 100;
    }

    [[nodiscard]] static T fromPercent(int pct, T min, T max) {
        return min + pct / 100. * (max - min);
    }

    void increment() override {
        value = std::min(value + incrementAmount, max);
    }

    void decrement() override {
        value = std::max(value - incrementAmount, min);
    }

    [[nodiscard]] static Numeric<float> makeZeroToOne(float initial) {
        if (initial < 0 || initial > 1) {
            throw std::out_of_range("initial must be between 0 and 1");
        }
        return {initial, 0, 1};
    }

    [[nodiscard]] static Numeric<float> makeFrequency(float initial) {
        if (initial < 20 || initial > 20000) {
            throw std::out_of_range("initial must be between 20 and 20,000");
        }
        return {initial, 20, 20000};
    }
};

class I_UIControls {
public:
    virtual ~I_UIControls() = default;

    [[nodiscard]] virtual I_Incrementable& currentControl() = 0;
    virtual void nextControl() = 0;
    virtual void previousControl() = 0;
};

template <typename Range>
struct SelectionInRange {
    SelectionInRange(std::shared_ptr<int> index, const Range& range) : _index{std::move(index)}, _range{range} {}

    [[nodiscard]] I_Incrementable& currentItem() {
        return *_range[*_index];
    }

    void nextItem() {
        if (*_index < _range.size() - 1) {
            (*_index)++;
        }
    }

    void previousItem() {
        if (*_index > 0) {
            (*_index)--;
        }
    }

private:
    std::shared_ptr<int> _index;
    Range _range;
};

struct State {
    // Controls that affect the synth.
    Numeric<float> attack;
    Numeric<float> decay;
    Numeric<float> sustain;
    Numeric<float> release;

    Numeric<float> sineWeight;
    Numeric<float> squareWeight;
    Numeric<float> triangleWeight;

    Numeric<float> gain;
    Numeric<float> lfoFrequency;
    Numeric<float> lfoGain;

    Numeric<float> delay_ms;
    Numeric<float> delayGain;

    Numeric<int> filterTypeIndex;

    Numeric<float> filterCutoffFrequency;
    Numeric<float> filterLfoDepth;
    Numeric<float> filterLfoFrequency;

    // UI-only state
    std::shared_ptr<int> selectedTab = std::make_shared<int>(0);
    std::shared_ptr<int> selectedOscillatorControl = std::make_shared<int>(0);
    std::shared_ptr<int> selectedOEnvelopeControl = std::make_shared<int>(0);
    std::shared_ptr<int> selectedEffectControl = std::make_shared<int>(0);

    bool showInfoMessage{true};
    bool isOscilloscopeLive{true};
    int oscilloscopeScaleFactorIndex{2};
    int oscilloscopeTimelineSizeIndex{2};

    [[nodiscard]] synth::TripleWeightsT getOscillatorWeights() const {
        return {sineWeight.value, squareWeight.value, triangleWeight.value};
    }

    [[nodiscard]] synth::ADSR getAdsr() const {
        return {attack.value, decay.value, sustain.value, release.value};
    }

    [[nodiscard]] std::size_t getDelay_samples(double sampleRate) const {
        return static_cast<std::size_t>(delay_ms.value / 1000 * sampleRate);
    }

    void resetControlSelections() {
        *selectedOscillatorControl = 0;
        *selectedOEnvelopeControl = 0;
        *selectedEffectControl = 0;
    }

    //! Applies any updated controls relevant to the synthesizer.
    void applyChanges(synth::Synthesizer& synth, const State& newControls) const {
        if (this->getOscillatorWeights() != newControls.getOscillatorWeights()) {
            synth.setOscillatorWeights(newControls.getOscillatorWeights());
        }

        if (this->gain != newControls.gain) {
            synth.setGain(newControls.gain.value);
        }

        if (this->lfoFrequency != newControls.lfoFrequency) {
            synth.setLfoFrequency(newControls.lfoFrequency.value);
        }

        if (this->lfoGain != newControls.lfoGain) {
            synth.setLfoGain(newControls.lfoGain.value);
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
            delay.setGain(newControls.delayGain.value);
        }
    }

    //! Applies any updated controls relevant to the modulated filter.
    void applyChanges(synth::ModulatedFilter& filter, const State& newControls) const {
        if (this->filterTypeIndex != newControls.filterTypeIndex) {
            filter.setFilterType(static_cast<synth::FilterType>(newControls.filterTypeIndex.value));
        }
        if (this->filterCutoffFrequency != newControls.filterCutoffFrequency) {
            filter.setCutoffFrequencyHz(newControls.filterCutoffFrequency.value);
        }
        if (this->filterLfoDepth != newControls.filterLfoDepth) {
            filter.setLFODepthHz(newControls.filterLfoDepth.value);
        }
        if (this->filterLfoFrequency != newControls.filterLfoFrequency) {
            filter.setLFOFrequencyHz(newControls.filterLfoFrequency.value);
        }
    }
};

}
