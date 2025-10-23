#include <synth/synthesizer.hpp>

#include <common/log.hpp>
#include <common/mutex_protected.hpp>
#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/low_frequency_oscillator.hpp>
#include <synth/oscillator.hpp>
#include <synth/voice.hpp>

#include <cmath>
#include <unordered_set>
#include <vector>

namespace synth {

namespace {

struct SynthesizerState {
    explicit SynthesizerState(const std::vector<WeightedWaveTable>& waveTables, const std::vector<Voice>& voices) :
        weightedWaveTables(waveTables),
        voices(voices),
        sustainPedalOn(false) {}

    void noteOn(int note, int velocity) {
        voices[note].setVelocity(velocity);
        voices[note].triggerOn();
        activeVoices.insert(note);
    }

    void noteOff(int note) {
        if (sustainPedalOn) {
            sustainedVoices.insert(note);
        } else {
            voices[note].triggerOff();
        }
    }

    void sustainOn() {
        sustainPedalOn = true;
    }

    void sustainOff() {
        sustainPedalOn = false;
        for (const auto& id : sustainedVoices) {
            voices[id].triggerOff();
        }
        sustainedVoices.clear();
    }

    //! TODO: Figure out where to call this.
    void clearActiveVoices() {
        for (const auto& id : activeVoices) {
            if (!voices[id].isActive()) {
                activeVoices.erase(id);
            }
        }
    }

    std::vector<WeightedWaveTable> weightedWaveTables;

    // Controller state.
    std::vector<Voice> voices;
    std::unordered_set<int> activeVoices;
    std::unordered_set<int> sustainedVoices;
    bool sustainPedalOn;
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

    std::vector<WeightedWaveTable> weightedWaveTables() const {
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

    float nextSample() {
        auto nextSample = 0.f;
        // Drops the sample instead of blocking for access to state.
        _state.getIfAvailable([&nextSample](SynthesizerState& state) {
            for (auto& id : state.activeVoices) {
                nextSample += state.voices.at(id).nextSample(state.weightedWaveTables);
            }
        });
        return nextSample;
    }

    void noteOn(int note, int velocity) {
        _state.get([&](SynthesizerState& state) {
            state.noteOn(note, velocity);
        });
    }

    void noteOff(int note) {
        _state.get([&](SynthesizerState& state) {
            state.noteOff(note);
        });
    }

    void sustainOn() {
        _state.get([](SynthesizerState& state) {
            state.sustainOn();
        });
    }

    void sustainOff() {
        _state.get([](SynthesizerState& state) {
            state.sustainOff();
        });
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

std::vector<WeightedWaveTable> Synthesizer::weightedWaveTables() const {
    return _impl->weightedWaveTables();
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

float Synthesizer::nextSample() {
    return _impl->nextSample();
}

void Synthesizer::noteOn(int note, int velocity) {
    _impl->noteOn(note, velocity);
}

void Synthesizer::noteOff(int note) {
    _impl->noteOff(note);
}

void Synthesizer::sustainOn() {
    _impl->sustainOn();
}

void Synthesizer::sustainOff() {
    _impl->sustainOff();
}

}
