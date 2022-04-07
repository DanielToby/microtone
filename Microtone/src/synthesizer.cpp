#include <microtone/synthesizer.hpp>
#include <microtone/envelope.hpp>
#include <microtone/exception.hpp>
#include <microtone/filter.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer_voice.hpp>
#include <microtone/oscillator.hpp>

#include <portaudio/portaudio.h>
#include <rtmidi/RtMidi.h>

#include <mutex>
#include <unordered_set>
#include <vector>


namespace microtone {

class Synthesizer::impl {
public:
    impl(OnOutputFn fn) :
        _onOutputFn{fn},
        _activeVoices{},
        _sustainedVoices{},
        _voices{},
        _sustainPedalOn{false},
        _sampleRate{0} {
        auto portAudioInitResult = Pa_Initialize();
        if (portAudioInitResult != paNoError) {
            throw MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 portAudioInitResult,
                                                 Pa_GetErrorText(portAudioInitResult)));
        }

        auto deviceId = Pa_GetDefaultOutputDevice();
        auto outputParameters = PaStreamParameters{};
        outputParameters.device = deviceId;
        if (outputParameters.device == paNoDevice) {
            throw MicrotoneException(fmt::format("Unable to open output device {}.", deviceId));
        }

        const auto deviceInfo = Pa_GetDeviceInfo(deviceId);
        if (deviceInfo) {
            M_INFO("Opened output device '{}'.", deviceInfo->name);
        } else {
            throw MicrotoneException(fmt::format("Unable to collect info on output device {}.", deviceId));
        }

        _sampleRate = deviceInfo->defaultSampleRate;

        outputParameters.channelCount = 2;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        auto envelope = Envelope{0.01, 0.1, .8, 0.01, _sampleRate};
        auto lowPassFilter = Filter{};

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
            throw MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 openStreamResult,
                                                 Pa_GetErrorText(openStreamResult)));
        }
        auto startStreamResult = Pa_StartStream(_portAudioStream);
        if (startStreamResult != paNoError) {
            throw MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 startStreamResult,
                                                 Pa_GetErrorText(startStreamResult)));
        }

        // Create voices ahead of time
        for (auto i = 0; i < 127; ++i) {
            // Sine wave oscillator
            auto oscillator = Oscillator{noteToFrequencyHertz(i), _sampleRate, [](WaveTable& table) {
                                             for (auto i = 0; i < static_cast<int>(table.size()); ++i) {
                                                 table[i] = std::sin(2.0 * M_PI * i / static_cast<int>(table.size()));
                                             }
                                         }};
            _voices.emplace_back(noteToFrequencyHertz(i), envelope, oscillator, lowPassFilter);
        }
    }

    ~impl() {
        if (_portAudioStream) {
            Pa_StopStream(_portAudioStream);
        }
        Pa_Terminate();
    }

    /* This routine will be called by the PortAudio engine when audio is needed.
    ** It may called at interrupt level on some machines so don't do anything
    ** that could mess up the system like calling malloc() or free().
    */
    static int paCallback([[maybe_unused]] const void *inputBuffer,
                          void *outputBuffer,
                          unsigned long framesPerBuffer,
                          [[maybe_unused]] const PaStreamCallbackTimeInfo* timeInfo,
                          [[maybe_unused]] PaStreamCallbackFlags statusFlags,
                          void *userData) {

            auto data = static_cast<impl*>(userData);
            auto out = static_cast<float*>(outputBuffer);

            for (auto frame = 0; frame < static_cast<int>(framesPerBuffer); ++frame) {
                auto sample = data->nextSample();
                data->_lastOutputBuffer[frame] = sample;

                for (std::size_t channel = 0; channel < 2; ++channel) {
                    *out++ = sample;
                }
            }

            data->_onOutputFn(data->_lastOutputBuffer);

            return paContinue;
    }

    float nextSample() {
        if (_voices.empty()) {
            return 0;
        }
        auto nextSample = 0.0;
        // Never block the audio thread.
        // And never, ever stop playing in the middle of a hoedown
        if (_mutex.try_lock()) {
            for (auto& id : _activeVoices) {
                nextSample += _voices[id].nextSample();
            }
            _mutex.unlock();
        }
        return nextSample;
    }

    void setEnvelope(const Envelope &envelope) {
        for (auto& voice : _voices) {
            voice.setEnvelope(envelope);
        }
    }

    void setFilter(const Filter &filter) {
        for (auto& voice : _voices) {
            voice.setFilter(filter);
        }
    }

    double noteToFrequencyHertz(int note) {
        constexpr auto pitch = 440.0f;
        return pitch * std::pow(2.0f, static_cast<float>(note - 69) / 12.0);
    }


    void addMidiData(int status, int note, int velocity) {
        auto lockGaurd = std::unique_lock<std::mutex>{_mutex};
        if (status == 0b10010000) {
            // Note on
            _voices[note].setVelocity(velocity);
            _voices[note].triggerOn();
            _activeVoices.insert(note);
        } else if (status == 0b10000000) {
            // Note off
            if (_sustainPedalOn) {
                _sustainedVoices.insert(note);
            } else {
                _voices[note].triggerOff();
            }
        } else if (status == 0b10110000) {
            // Control Change
            if (note == 64) {
                _sustainPedalOn = velocity > 64;
                if (!_sustainPedalOn) {
                    for (const auto& id : _sustainedVoices) {
                        _voices[id].triggerOff();
                    }
                    _sustainedVoices.clear();
                }
            }
        }

        for (const auto& id : _activeVoices) {
            if (!_voices[id].isActive()) {
                _activeVoices.erase(id);
            }
        }
    }

    double sampleRate() {
        return _sampleRate;
    }

    OnOutputFn _onOutputFn;
    PaStream* _portAudioStream;
    std::mutex _mutex;
    std::array<float, FRAMES_PER_BUFFER> _lastOutputBuffer; // Forwarded to onOutputFn()
    std::unordered_set<int> _activeVoices;
    std::unordered_set<int> _sustainedVoices;
    std::vector<SynthesizerVoice> _voices;
    bool _sustainPedalOn;
    double _sampleRate;
};

Synthesizer::Synthesizer(OnOutputFn fn) :
    _impl{new impl{fn}} {
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

void Synthesizer::setEnvelope(const Envelope &envelope) {
    _impl->setEnvelope(envelope);
}

void Synthesizer::setFilter(const Filter &filter) {
    _impl->setFilter(filter);
}

void Synthesizer::addMidiData(int status, int note, int velocity) {
    _impl->addMidiData(status, note, velocity);
}

double Synthesizer::sampleRate() {
    return _impl->sampleRate();
}

}
