#include <microtone/envelope.hpp>
#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer_voice.hpp>
#include <microtone/oscillator.hpp>
#include <microtone/synthesizer.hpp>

#include <rtmidi/RtMidi.h>

#include <chrono>
#include <mutex>
#include <unordered_set>

#include <audio>

namespace microtone {

class Synthesizer::impl {
public:
    impl() :
        _currentNotes{} {
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

        _envelope = Envelope{0.01, 0.1, .8, 0.01, _sampleRate};
        _oscillator = Oscillator{WaveType::Sine, _sampleRate, 5, 1.0};
        _outputDevice->start();
    }

    double noteToFrequencyHertz(int note) {
        constexpr auto pitch = 440.0f;
        return pitch * std::pow(2.0f, float(note - 69) / 12.0);
    }

    void addNoteData(int note, int velocity, bool isPressed) {
        auto lockGaurd = std::unique_lock<std::mutex>{_mutex};

        if (_currentNotes.find(note) != _currentNotes.end()) {
            if (isPressed) {
                _currentNotes[note].velocity = velocity;
                _currentNotes[note].envelope.triggerOn();
            } else {
                _currentNotes[note].envelope.triggerOff();
            }
        } else {
            if (isPressed) {
                auto synthNote = SynthesizerVoice{noteToFrequencyHertz(note), velocity, _envelope, _oscillator};
                synthNote.envelope.triggerOn();
                _currentNotes[note] = std::move(synthNote);
            }
        }
    }

    float nextSample() {
        if (_currentNotes.empty()) {
            return 0;
        }
        auto nextSample = 0.0;
        // Never block the audio thread.
        // And never, ever stop playing in the middle of a hoedown
        if (_mutex.try_lock()) {
            for (auto& [id, synthNote] : _currentNotes) {
                if (synthNote.envelope.state() == EnvelopeState::Off) {
                    _currentNotes.erase(id);
                } else {
                    nextSample += synthNote.nextSample();
                }
            }
            _mutex.unlock();
        }

        return nextSample;
    }

    std::optional<std::experimental::audio_device> _outputDevice;
    std::mutex _mutex;
    std::unordered_map<int, SynthesizerVoice> _currentNotes;
    Envelope _envelope;
    Oscillator _oscillator;
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
