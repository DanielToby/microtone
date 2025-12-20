#pragma once

#include <common/ring_buffer.hpp>

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
    explicit AudioOutputStream(std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> inputBuffer);
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

    [[nodiscard]] double sampleRate() const;

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}