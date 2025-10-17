#include <common/exception.hpp>
#include <common/log.hpp>
#include <io/midi_input.hpp>
#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/low_frequency_oscillator.hpp>
#include <synth/midi_device.hpp>
#include <synth/oscillator.hpp>
#include <synth/synthesizer.hpp>
#include <synth/voice.hpp>

#include <portaudio.h>
#include <RtMidi.h>

#include <cmath>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace synth {

class Synthesizer::impl {
public:
    impl(const std::vector<WeightedWaveTable>& weightedWaveTables, OnOutputFn fn) :
        _onOutputFn{fn},
        _weightedWaveTables{weightedWaveTables},
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

        auto voices = std::vector<Voice>{};
        for (auto i = 0; i < 127; ++i) {
            voices.emplace_back(
                noteToFrequencyHertz(i),
                Envelope{ADSR{0.01, 0.1, .8, 0.01}, _sampleRate},
                Oscillator{noteToFrequencyHertz(i), _sampleRate},
                LowFrequencyOscillator{0.25, _sampleRate},
                Filter{}
            );
        }

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

        data->_onOutputFn(data->_lastOutputBuffer);

        return paContinue;
    }

    float nextSample() {
        auto nextSample = 0.f;
        if (_mutex.try_lock()) {
            for (auto& id : _device.activeVoices) {
                nextSample += _device.voices[id].nextSample(_weightedWaveTables);
            }
            _mutex.unlock();
        }
        return .0;
    }

    std::vector<WeightedWaveTable> weightedWaveTables() const {
        return _weightedWaveTables;
    }

    void setWaveTables(const std::vector<WeightedWaveTable>& weightedWaveTables) {
        auto lockGuard = std::unique_lock<std::mutex>{_mutex};
        _weightedWaveTables = weightedWaveTables;
    }

    void setEnvelope(const Envelope& envelope) {
        auto lockGuard = std::unique_lock<std::mutex>{_mutex};
        for (auto& voice : _device.voices) {
            voice.setEnvelope(envelope);
        }
    }

    void setFilter(const Filter& filter) {
        auto lockGuard = std::unique_lock<std::mutex>{_mutex};
        for (auto& voice : _device.voices) {
            voice.setFilter(filter);
        }
    }

    double noteToFrequencyHertz(int note) {
        constexpr auto pitch = 440.0f;
        return pitch * std::pow(2.0f, static_cast<float>(note - 69) / 12.0);
    }

    void submitMidiMessage(const MidiMessage& midiMessage) {
        auto lockGuard = std::unique_lock<std::mutex>{_mutex};
        switch (static_cast<io::MidiStatusMessage>(midiMessage.status)) {
            case io::MidiStatusMessage::NoteOn:
                _device.noteOn(midiMessage.note, midiMessage.velocity);
            case io::MidiStatusMessage::NoteOff:
                _device.noteOff(midiMessage.note);
            case io::MidiStatusMessage::ControlChange: {
                switch (static_cast<io::MidiNoteMessage>(midiMessage.note)) {
                    case io::MidiNoteMessage::SustainPedal: {
                        _device.setSustain(midiMessage.velocity > static_cast<int>(io::MidiVelocityMessage::SustainPedalOnThreshold));
                    }
                }
            }
        }
        const auto activeVoices = _device.activeVoices;
        for (const auto& id : activeVoices) {
            if (!_device.voices[id].isActive()) {
                _device.activeVoices.erase(id);
            }
        }
    }

    double sampleRate() {
        return _sampleRate;
    }

    OnOutputFn _onOutputFn;
    PaStream* _portAudioStream;    // Owned by port audio, cleaned up by Pa_Terminate().
    std::mutex _mutex;
    std::vector<WeightedWaveTable> _weightedWaveTables;
    MidiDevice _device;
    AudioBuffer _lastOutputBuffer;  // Forwarded to onOutputFn()
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

void Synthesizer::submitMidiMessage(int status, int note, int velocity) {
    _impl->submitMidiMessage(MidiMessage{status, note, velocity});
}

double Synthesizer::sampleRate() {
    return _impl->sampleRate();
}

}
