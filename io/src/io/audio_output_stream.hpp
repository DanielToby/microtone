#pragma once

#include <memory>

namespace io {

enum class AudioStreamError {
    NoError = 0,
    InitializationFailed,
    NoDevice,
    NoDeviceInfo,
    OpenStreamError,
    StartStreamError,
    StopStreamError,
};

//! This is the portaudio wrapper.
class AudioOutputStream {
public:
    AudioOutputStream();
    AudioOutputStream(const AudioOutputStream&) = delete;
    AudioOutputStream& operator=(const AudioOutputStream&) = delete;
    AudioOutputStream(AudioOutputStream&&) noexcept;
    AudioOutputStream& operator=(AudioOutputStream&&) noexcept;
    ~AudioOutputStream();

    //! Callers should check this after construction...
    //! TODO: Factory fn returning std::expected<AudioOutputStream, OpenStreamError>
    [[nodiscard]] AudioStreamError createStreamError() const;

    void start();
    void stop();
    void push(float sample);

    [[nodiscard]] double sampleRate() const;

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}