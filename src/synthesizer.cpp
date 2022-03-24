#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer.hpp>

#include <rtmidi/RtMidi.h>

#include <audio>

namespace microtone {

class Synthesizer::impl {
public:
    impl() {
        _outputDevice = std::experimental::get_default_audio_output_device();
        if (!_outputDevice) {
            throw MicrotoneException("The default audio output device could not be identified.");
        }

        _sampleRate = _outputDevice->get_sample_rate();
        if (_sampleRate == 0) {
            throw MicrotoneException("The device sample rate could not be identified.");
        }

        _outputDevice->connect([this](std::experimental::audio_device&, std::experimental::audio_device_io<float>& io) mutable noexcept {
            if (!io.output_buffer.has_value())
                return;

            auto& out = *io.output_buffer;

            for (std::size_t frame = 0; frame < out.size_frames(); ++frame) {
                auto sample = nextSample();

                for (std::size_t channel = 0; channel < out.size_channels(); ++channel) {
                    out(frame, channel) = sample;
                }
            }
        });

        _outputDevice->start();
    }

    float noteToFrequencyHertz(int note) {
        constexpr auto pitch = 440.0f;
        return pitch * std::pow(2.0f, float(note - 69) / 12.0f);
    }

    void update() noexcept {
        auto frequency = noteToFrequencyHertz(_currentNote);
        _delta = 2.0f * frequency * static_cast<float>(M_PI / _sampleRate);
    }

    float nextSample() {
        if (_currentNote != -1) {
            update();
        } else {
            return 0;
        }

        auto next_sample = std::copysign(0.1f, std::sin(_phase));
        _phase = std::fmod(_phase + _delta, 2.0f * float(M_PI));
        return next_sample;
    }

    void setNote(int note) {
        if (note == _currentNote) {
            _currentNote = -1;
        } else if (0 <= note && note <= 127) {
            _currentNote = note;
        }
    }

    std::optional<std::experimental::audio_device> _outputDevice;
    int _currentNote = -1;
    float _sampleRate = 0;
    float _delta = 0;
    float _phase = 0;
};

Synthesizer::Synthesizer() :
    _impl{new impl{}} {
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

void Synthesizer::setNote(int note) {
    _impl->setNote(note);
}

}
