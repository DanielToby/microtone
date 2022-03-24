#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer.hpp>
#include <microtone/oscillator.hpp>
#include <microtone/midi_note.hpp>

#include <rtmidi/RtMidi.h>

#include <unordered_map>

#include <audio>

namespace microtone {

class Synthesizer::impl {
public:
    impl() : _currentNotes{} {
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

    double noteToFrequencyHertz(int note) {
        constexpr auto pitch = 440.0f;
        return pitch * std::pow(2.0f, float(note - 69) / 12.0);
    }

    double velocityToAmplitude(int velocity) {
        // desired dynamic range: 60db
        const auto r = std::pow(10, 60 / 20);
        const auto b = 127 / (126 * sqrt(r)) - 1/126;
        const auto m = (1 - b) / 127;
        return std::pow(m * velocity + b, 2);
    }

    float nextSample() {
        if (_currentNotes.empty()) {
            return 0;
        }

        auto nextSample = 0.0;
        for (auto& [note, data] : _currentNotes) {
            auto frequency = noteToFrequencyHertz(note);
            data.second.delta = 2.0f * frequency * static_cast<float>(M_PI / _sampleRate);
            nextSample += velocityToAmplitude(data.first.velocity) * std::copysign(0.1f, std::sin(data.second.phase));
            data.second.phase = std::fmod(data.second.phase + data.second.delta, 2.0f * float(M_PI));
        }
        return nextSample;
    }

    void addNoteData(int note, int velocity, double timeStamp) {
        if (_currentNotes.find(note) != _currentNotes.end()) {
            _currentNotes[note].first.finishTimeStamp = timeStamp;

            // TODO: calculate isActive with envelope.
            _currentNotes.erase(note);
        } else {
            _currentNotes[note] = std::make_pair<MidiNote, Oscillator>(MidiNote{note, velocity, timeStamp}, Oscillator{});
        }
    }

    std::optional<std::experimental::audio_device> _outputDevice;
    std::unordered_map<int, std::pair<MidiNote, Oscillator>> _currentNotes;
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

void Synthesizer::addNoteData(int note, int velocity, double timeStamp) {
    _impl->addNoteData(note, velocity, timeStamp);
}

}
