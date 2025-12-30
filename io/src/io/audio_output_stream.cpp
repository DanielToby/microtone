#include <io/audio_output_stream.hpp>

#include <portaudio.h>

#include <common/exception.hpp>
#include <common/log.hpp>
#include <common/ring_buffer.hpp>
#include <common/timer.hpp>

namespace io {

class AudioOutputStream::impl {
public:
    explicit impl(std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> outputBuffer) :
        _outputBuffer{std::move(outputBuffer)},
        _portAudioStream{nullptr},
        _sampleRate{0},
        _createStreamError{AudioStreamError::NoError} {

        if (auto initResult = Pa_Initialize(); initResult != paNoError) {
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
        _sampleRate = deviceInfo->defaultSampleRate;

        auto outputParameters = PaStreamParameters{
            /* device */ deviceId,
            /* channelCount */ 1,
            /* sampleFormat */ paFloat32,
            /* suggestedLatency */ deviceInfo->defaultLowOutputLatency,
            /* hostApiSpecificStreamInfo */ nullptr};

        auto openStreamResult = Pa_OpenStream(
            &_portAudioStream,
            nullptr,
            &outputParameters,
            _sampleRate,
            common::audio::AudioBlockSize,
            paNoFlag,
            &portAudioCallback,
            _outputBuffer.get());

        if (openStreamResult != paNoError) {
            M_ERROR(fmt::format("ERROR: {}", static_cast<int>(openStreamResult)));
            _createStreamError = AudioStreamError::OpenStreamError;
        }
    }

    ~impl() {
        if (_portAudioStream) {
            Pa_StopStream(_portAudioStream);
        }
        Pa_Terminate();
    }

    static int portAudioCallback(const void* /*input*/,
                                 void* outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* /*timeInfo*/,
                                 PaStreamCallbackFlags /*statusFlags*/,
                                 void* rawUserData) {
        auto* userData = static_cast<common::RingBuffer<common::audio::FrameBlock>*>(rawUserData);
        auto* out = static_cast<float*>(outputBuffer);
        if (!userData || !out) {
            return paContinue;
        }

        auto addData = [&out](const common::audio::FrameBlock& block) {
            for (const auto& v : block) {
                *out++ = v;
            }
        };

        if (!userData->pop(addData)) {
            M_ERROR(fmt::format("Dropped frame!"));
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

    [[nodiscard]] double sampleRate() const {
        return _sampleRate;
    }

    std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> _outputBuffer;
    PaStream* _portAudioStream;
    double _sampleRate;
    AudioStreamError _createStreamError;
};

AudioOutputStream::AudioOutputStream(std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> inputBuffer) :
    _impl{std::make_unique<impl>(inputBuffer)} {
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

double AudioOutputStream::sampleRate() const {
    if (this->createStreamError() != AudioStreamError::NoError) {
        throw common::MicrotoneException("Audio output stream initialization failed.");
    }
    return _impl->sampleRate();
}

}
