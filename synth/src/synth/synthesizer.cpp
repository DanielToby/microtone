#include <synth/synthesizer.hpp>

#include <common/exception.hpp>
#include <common/log.hpp>
#include <common/mutex_protected.hpp>
#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/low_frequency_oscillator.hpp>
#include <synth/oscillator.hpp>
#include <synth/voice.hpp>

#include <portaudio.h>

#include <cmath>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace synth {

namespace {

//! TODO: break this up.
struct SynthesizerState {
    //! TODO: Remove this when sampleRate is passed to Synthesizer c'tor.
    SynthesizerState() = default;

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
    impl(const std::vector<WeightedWaveTable>& weightedWaveTables, OnOutputFn fn) :
        _onOutputFn{fn},
        _sampleRate{0} {
        // Initialize portaudio
        auto portAudioInitResult = Pa_Initialize();
        if (portAudioInitResult != paNoError) {
            throw common::MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 portAudioInitResult,
                                                 Pa_GetErrorText(portAudioInitResult)));
        }

        auto deviceId = Pa_GetDefaultOutputDevice();
        auto outputParameters = PaStreamParameters{};
        outputParameters.device = deviceId;
        if (outputParameters.device == paNoDevice) {
            throw common::MicrotoneException(fmt::format("Unable to open output device {}.", deviceId));
        }

        const auto deviceInfo = Pa_GetDeviceInfo(deviceId);
        if (deviceInfo) {
            M_INFO("Opened output device '{}'.", deviceInfo->name);
        } else {
            throw common::MicrotoneException(fmt::format("Unable to collect info on output device {}.", deviceId));
        }

        _sampleRate = deviceInfo->defaultSampleRate;

        outputParameters.channelCount = 2;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        PaError openStreamResult = Pa_OpenStream(
            &_portAudioStream,
            nullptr,
            &outputParameters,
            _sampleRate,
            FRAMES_PER_BUFFER,
            paClipOff,
            &impl::paCallback,
            this);

        if (openStreamResult != paNoError) {
            throw common::MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 openStreamResult,
                                                 Pa_GetErrorText(openStreamResult)));
        }

        // This can move into the initializer list when sample rate is passed.
        _state = common::MutexProtected{SynthesizerState{weightedWaveTables, buildVoices(_sampleRate, ADSR{0.01, 0.1, .8, 0.01}, .25)}};
    }

    ~impl() {
        if (_portAudioStream) {
            Pa_StopStream(_portAudioStream);
        }
        Pa_Terminate();
    }

    void start() {
        auto startStreamResult = Pa_StartStream(_portAudioStream);
        if (startStreamResult != paNoError) {
            throw common::MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 startStreamResult,
                                                 Pa_GetErrorText(startStreamResult)));
        }
        M_INFO("Started synthesizer.");
    }

    void stop() {
        auto stopStreamResult = Pa_StopStream(_portAudioStream);
        if (stopStreamResult != paNoError) {
            throw common::MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 stopStreamResult,
                                                 Pa_GetErrorText(stopStreamResult)));
        }
        M_INFO("Stopped synthesizer.");
    }

    /* This routine will be called by the PortAudio engine when audio is needed.
       It may called at interrupt level on some machines so don't do anything
       that could mess up the system like calling malloc() or free().
    */
    static int paCallback([[maybe_unused]] const void* inputBuffer,
                          void* outputBuffer,
                          unsigned long framesPerBuffer,
                          [[maybe_unused]] const PaStreamCallbackTimeInfo* timeInfo,
                          [[maybe_unused]] PaStreamCallbackFlags statusFlags,
                          void* userData) {
        auto data = static_cast<impl*>(userData);
        auto out = static_cast<float*>(outputBuffer);

        for (auto frame = 0; frame < static_cast<int>(framesPerBuffer); ++frame) {
            auto sample = data->nextSample();

            data->_lastOutputBuffer[frame] = sample;

            for (std::size_t channel = 0; channel < 2; ++channel) {
                *out++ = sample;
            }
        }

        // TODO: expose this in a way that doesn't block the audio thread.
        data->_onOutputFn(data->_lastOutputBuffer);

        return paContinue;
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

    double sampleRate() {
        return _sampleRate;
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

    AudioBuffer _lastOutputBuffer;

    OnOutputFn _onOutputFn;
    PaStream* _portAudioStream;    // Owned by port audio, cleaned up by Pa_Terminate().
    common::MutexProtected<SynthesizerState> _state;
    double _sampleRate;
};

Synthesizer::Synthesizer(const std::vector<WeightedWaveTable>& weightedWaveTables, OnOutputFn fn) :
    _impl{std::make_unique<impl>(weightedWaveTables, fn)} {
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

void Synthesizer::start() {
    _impl->start();
}

void Synthesizer::stop() {
    _impl->stop();
}

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

double Synthesizer::sampleRate() {
    return _impl->sampleRate();
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
