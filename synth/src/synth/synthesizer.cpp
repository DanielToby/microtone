#include <synth/synthesizer.hpp>

#include <common/mutex_protected.hpp>
#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/low_frequency_oscillator.hpp>
#include <synth/oscillator.hpp>
#include <synth/voice.hpp>

#include <cmath>
#include <vector>

namespace synth {

namespace {

//! These things are modifiable while the synth is active.
struct SynthesizerState {
    std::vector<WeightedWaveTable> weightedWaveTables;
    std::vector<Voice> voices;
};

[[nodiscard]] double noteToFrequencyHertz(int note) {
    constexpr auto pitch = 440.0f;
    return pitch * std::pow(2.0f, static_cast<float>(note - 69) / 12.0);
}

[[nodiscard]] std::vector<Voice> buildVoices(double sampleRate, const ADSR& adsr, double lfoFrequencyHertz) {
    auto result = std::vector<Voice>{};
    for (auto i = 0; i < 127; ++i) {
        result.emplace_back(
            noteToFrequencyHertz(i),
            Envelope{adsr, sampleRate},
            Oscillator{noteToFrequencyHertz(i), sampleRate},
            LowFrequencyOscillator{lfoFrequencyHertz, sampleRate},
            Filter{});
    }
    return result;
}

}

class Synthesizer::impl {
public:
    impl(double sampleRate, const std::vector<WeightedWaveTable>& waveTables) :
        _state{{SynthesizerState{waveTables, buildVoices(sampleRate, ADSR{0.01, 0.1, .8, 0.01}, .25)}}} {}

    ~impl() {}

    std::vector<WeightedWaveTable> waveTables() const {
        return _state.get([](const SynthesizerState& state) {
            return state.weightedWaveTables;
        });
    }

    void setWaveTables(const std::vector<WeightedWaveTable>& weightedWaveTables) {
        _state.get([&](SynthesizerState& state) {
            state.weightedWaveTables = weightedWaveTables;
        });
    }

    void setEnvelope(const Envelope& envelope) {
        _state.get([&](SynthesizerState& state) {
            for (auto& voice : state.voices) {
                voice.setEnvelope(envelope);
            }
        });
    }

    void setFilter(const Filter& filter) {
        _state.get([&](SynthesizerState& state) {
            for (auto& voice : state.voices) {
                voice.setFilter(filter);
            }
        });
    }

    std::optional<float> nextSample(const common::midi::Keyboard& keyboard) {
        auto nextSample = std::optional<float>{};
        _state.getIfAvailable([&](SynthesizerState& state) {
            nextSample = 0.0f;
            for (auto i = 0; i < keyboard.notes.size(); ++i) {
                const auto& note = keyboard.notes[i];
                auto& voice  = state.voices[i];
                if (note.isOn()) {
                    voice.triggerOn();
                } else if (note.isOff()) {
                    voice.triggerOff();
                }

                *nextSample += voice.nextSample(state.weightedWaveTables);
            }
        });
        return nextSample;
    }

    common::MutexProtected<SynthesizerState> _state;
};

Synthesizer::Synthesizer(double sampleRate, const std::vector<WeightedWaveTable>& waveTables) :
    _impl{std::make_unique<impl>(sampleRate, waveTables)} {
}

Synthesizer::Synthesizer(Synthesizer&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Synthesizer& Synthesizer::operator=(Synthesizer&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

Synthesizer::~Synthesizer() = default;

std::vector<WeightedWaveTable> Synthesizer::waveTables() const {
    return _impl->waveTables();
}

void Synthesizer::setWaveTables(const std::vector<WeightedWaveTable>& weightedWaveTables) {
    _impl->setWaveTables(weightedWaveTables);
}

void Synthesizer::setEnvelope(const Envelope& envelope) {
    _impl->setEnvelope(envelope);
}

void Synthesizer::setFilter(const Filter& filter) {
    _impl->setFilter(filter);
}

std::optional<float> Synthesizer::nextSample(const common::midi::Keyboard& keyboard) {
    return _impl->nextSample(keyboard);
}

}
