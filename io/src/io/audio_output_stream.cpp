#include <io/audio_output_stream.hpp>

#include <portaudio.h>

#include <common/exception.hpp>
#include <common/log.hpp>
#include <io/ring_buffer.hpp>

namespace io {

namespace {

const int FRAMES_PER_BUFFER = 512;

}

class AudioOutputStream::impl {
public:
    impl() : _buffer{}, _portAudioStream{nullptr}, _sampleRate{0.f}, _createStreamError{AudioStreamError::NoError} {
        if (auto initResult = Pa_Initialize(); initResult == paNoError) {
            _createStreamError = AudioStreamError::InitializationFailed;
            return;
        }

        auto deviceId = Pa_GetDefaultOutputDevice();
        if (deviceId == paNoDevice) {
            _createStreamError = AudioStreamError::NoDevice;
            return;
        }

        const auto deviceInfo = Pa_GetDeviceInfo(deviceId);
        if (!deviceInfo) {
            _createStreamError = AudioStreamError::NoDeviceInfo;
            return;
        }

        auto outputParameters = PaStreamParameters{
            /* device */ deviceId,
            /* channelCount */ 2,
            /* sampleFormat */ paFloat32,
            /* suggestedLatency */ deviceInfo->defaultLowOutputLatency,
            /* hostApiSpecificStreamInfo */ nullptr
        };

        auto openStreamResult = Pa_OpenStream(
            &_portAudioStream,
            nullptr,
            &outputParameters,
            _sampleRate,
            FRAMES_PER_BUFFER,
            paClipOff,
            &impl::portAudioCallback,
            &_buffer);

        if (openStreamResult != paNoError) {
            _createStreamError = AudioStreamError::OpenStreamError;
        }
    }

    ~impl() {
        if (_portAudioStream) {
            Pa_StopStream(_portAudioStream);
        }
        Pa_Terminate();
    }

    static int portAudioCallback([[maybe_unused]] const void* inputBuffer,
                                 void* outputBuffer,
                                 unsigned long framesPerBuffer,
                                 [[maybe_unused]] const PaStreamCallbackTimeInfo* timeInfo,
                                 [[maybe_unused]] PaStreamCallbackFlags statusFlags,
                                 void* userData) {
        auto dataBuffer = static_cast<RingBuffer<float, FRAMES_PER_BUFFER>*>(userData);
        auto out = static_cast<float*>(outputBuffer);
        if (dataBuffer && out) {
            for (auto frame = 0; frame < static_cast<int>(framesPerBuffer); ++frame) {
                auto sample = dataBuffer->pop();
                for (std::size_t channel = 0; channel < 2; ++channel) {
                    *out++ = sample.value_or(0.f);
                }
            }
        } else {
            throw common::MicrotoneException("Invalid Port Audio callback.");
        }
        return paContinue;
    }

    [[nodiscard]] AudioStreamError createStreamError() const {
        return _createStreamError;
    }

    void start() {
        if (auto startStreamResult = Pa_StartStream(_portAudioStream); startStreamResult != paNoError) {
            throw common::MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 startStreamResult,
                                                 Pa_GetErrorText(startStreamResult)));
        }
        M_INFO("Started audio output stream.");
    }

    void stop() {
        if (auto stopStreamResult = Pa_StopStream(_portAudioStream); stopStreamResult != paNoError) {
            throw common::MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 stopStreamResult,
                                                 Pa_GetErrorText(stopStreamResult)));
        }
        M_INFO("Stopped audio output stream.");
    }

    void push(float sample) {
        _buffer.push(sample);
    }

    [[nodiscard]] double sampleRate() const {
        return _sampleRate;
    }

    RingBuffer<float, FRAMES_PER_BUFFER> _buffer;
    PaStream* _portAudioStream;
    double _sampleRate;
    AudioStreamError _createStreamError;
};

AudioOutputStream::AudioOutputStream() :
    _impl{std::make_unique<impl>()} {
}

AudioOutputStream::AudioOutputStream(AudioOutputStream&& other) noexcept :
    _impl{std::move(other._impl)} {
}

AudioOutputStream& AudioOutputStream::operator=(AudioOutputStream&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

AudioOutputStream::~AudioOutputStream() = default;

AudioStreamError AudioOutputStream::createStreamError() const {
    return _impl->createStreamError();
}

void AudioOutputStream::start() {
    if (this->createStreamError() != AudioStreamError::NoError) {
        throw common::MicrotoneException("Audio output stream initialization failed.");
    }
    _impl->start();
}

void AudioOutputStream::stop() {
    if (this->createStreamError() != AudioStreamError::NoError) {
        throw common::MicrotoneException("Audio output stream initialization failed.");
    }
    _impl->stop();
}

void AudioOutputStream::push(float sample) {
    if (this->createStreamError() != AudioStreamError::NoError) {
        throw common::MicrotoneException("Audio output stream initialization failed.");
    }
    _impl->push(sample);
}

double AudioOutputStream::sampleRate() const {
    if (this->createStreamError() != AudioStreamError::NoError) {
        throw common::MicrotoneException("Audio output stream initialization failed.");
    }
    return _impl->sampleRate();
}

}
