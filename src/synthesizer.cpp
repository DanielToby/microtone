#include <microtone/envelope.hpp>
#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer_voice.hpp>
#include <microtone/oscillator.hpp>
#include <microtone/synthesizer.hpp>

#include <rtmidi/RtMidi.h>

#include <mutex>
#include <unordered_set>
#include <vector>

#include <audio>

namespace microtone {

class Synthesizer::impl {
public:
    impl() :
        _voices{} {
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

        auto envelope = Envelope{0.01, 0.1, .8, 0.01, _sampleRate};

        // Create voices ahead of time
        for (auto i = 0; i < 127; ++i) {
            auto oscillator = Oscillator{WaveType::Sine, noteToFrequencyHertz(i), _sampleRate};
            _voices.emplace_back(noteToFrequencyHertz(i), envelope, oscillator);
        }

        _outputDevice->start();
    }

    double noteToFrequencyHertz(int note) {
        constexpr auto pitch = 440.0f;
        return pitch * std::pow(2.0f, static_cast<float>(note - 69) / 12.0);
    }

    void addNoteData(int note, int velocity, bool isPressed) {
        auto lockGaurd = std::unique_lock<std::mutex>{_mutex};
        if (isPressed) {
            _voices[note].setVelocity(velocity);
            _voices[note].triggerOn();
            _activeVoices.insert(note);
        } else {
            _voices[note].triggerOff();
        }
        for (const auto& id : _activeVoices) {
            if (!_voices[id].isActive()) {
                _activeVoices.erase(id);
            }
        }
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

    std::optional<std::experimental::audio_device> _outputDevice;
    std::mutex _mutex;
    std::unordered_set<int> _activeVoices;
    std::vector<SynthesizerVoice> _voices;
    double _sampleRate;
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

void Synthesizer::addNoteData(int note, int velocity, bool isPressed) {
    _impl->addNoteData(note, velocity, isPressed);
}

}
