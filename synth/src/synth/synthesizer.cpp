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
    common::midi::Keyboard keyboard;
    std::optional<common::audio::FrameBlock> lastBlock;
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

//! Updates the voice (triggers it on or off) based on whether it was turned on or off.
void triggerVoiceIfNecessary(Voice& voice, const common::midi::Note& previousNote, const common::midi::Note& currentNote) {
    if (previousNote.isOff() && currentNote.isOn()) {
        voice.triggerOn(currentNote.velocity);
    } else if (previousNote.isOn() && currentNote.isOff()) {
        voice.triggerOff();
    }
}

//! Samples each voice in the provided state, using latestKeyboard.
[[nodiscard]] float nextSample(SynthesizerState& state) {
    auto result = 0.0f;
    for (auto i = 0; i < state.voices.size(); ++i) {
        auto& voice  = state.voices[i];
        result += voice.nextSample(state.weightedWaveTables);
    }
    return result;
}

}

class Synthesizer::impl {
public:
    impl(double sampleRate, const std::vector<WeightedWaveTable>& waveTables) :
        _state{{SynthesizerState{waveTables, buildVoices(sampleRate, ADSR{0.01, 0.1, .8, 0.01}, .25)}}},
        _sampleRate(sampleRate) {}

    ~impl() = default;

    std::vector<WeightedWaveTable> waveTables() const {
        return _state.read().weightedWaveTables;
    }

    void setWaveTables(const std::vector<WeightedWaveTable>& weightedWaveTables) {
        _state.write([&](SynthesizerState& state) {
            state.weightedWaveTables = weightedWaveTables;
        });
    }

    void setEnvelope(const Envelope& envelope) {
        _state.write([&](SynthesizerState& state) {
            for (auto& voice : state.voices) {
                voice.setEnvelope(envelope);
            }
        });
    }

    void setFilter(const Filter& filter) {
        _state.write([&](SynthesizerState& state) {
            for (auto& voice : state.voices) {
                voice.setFilter(filter);
            }
        });
    }

    void respondToKeyboardChanges(const common::midi::Keyboard& latestKeyboard) {
        const auto currentKeyboard = _state.read().keyboard;
        if (latestKeyboard != currentKeyboard) {
            _state.write([&latestKeyboard](SynthesizerState& state) {
                for (auto i = 0; i < latestKeyboard.audibleNotes.size(); ++i) {
                    auto& previousNote = state.keyboard.audibleNotes[i];
                    const auto& currentNote = latestKeyboard.audibleNotes[i];

                    auto& voice = state.voices[i];
                    triggerVoiceIfNecessary(voice, previousNote, currentNote);
                    previousNote = currentNote;
                }
            });
        }
    }

    [[nodiscard]] common::audio::FrameBlock getNextBlock() {
        auto result = common::audio::FrameBlock{};
        _state.write([&result](SynthesizerState& state) {
            for (auto i = 0; i < result.size(); ++i) {
                result[i] = nextSample(state);
            }
            state.lastBlock = result;
        });
        return result;
    }

    [[nodiscard]] std::optional<common::audio::FrameBlock> getLastBlock() const {
        return _state.read().lastBlock;
    }

    [[nodiscard]] double sampleRate() const {
        return _sampleRate;
    }

private:
    common::MutexProtected<SynthesizerState> _state;
    double _sampleRate = -1;
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

void Synthesizer::respondToKeyboardChanges(const common::midi::Keyboard& keyboard) {
    _impl->respondToKeyboardChanges(keyboard);
}

common::audio::FrameBlock Synthesizer::getNextBlock() {
    return _impl->getNextBlock();
}

std::optional<common::audio::FrameBlock> Synthesizer::getLastBlock() const {
    return _impl->getLastBlock();
}

double Synthesizer::sampleRate() const {
    return _impl->sampleRate();
}

}
