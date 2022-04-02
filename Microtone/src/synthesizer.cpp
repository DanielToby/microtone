#include <microtone/synthesizer.hpp>
#include <microtone/envelope.hpp>
#include <microtone/exception.hpp>
#include <microtone/filter.hpp>
#include <microtone/log.hpp>
#include <microtone/synthesizer_voice.hpp>
#include <microtone/oscillator.hpp>

#include <rtmidi/RtMidi.h>

#include <mutex>
#include <unordered_set>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wreorder-ctor"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <audio>
#pragma GCC diagnostic pop

namespace microtone {

class Synthesizer::impl {
public:
    impl() :
        _activeVoices{},
        _sustainedVoices{},
        _voices{},
        _sustainPedalOn{false},
        _sampleRate{0} {
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
        auto lowPassFilter = Filter{};

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

        _outputDevice->start();
    }

    double noteToFrequencyHertz(int note) {
        constexpr auto pitch = 440.0f;
        return pitch * std::pow(2.0f, static_cast<float>(note - 69) / 12.0);
    }

    void addNoteData(int status, int note, int velocity) {
//        auto lockGaurd = std::unique_lock<std::mutex>{_mutex};
        _mutex.lock();
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
        _mutex.unlock();
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
    std::unordered_set<int> _sustainedVoices;
    std::vector<SynthesizerVoice> _voices;
    bool _sustainPedalOn;
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

void Synthesizer::addNoteData(int status, int note, int velocity) {
    _impl->addNoteData(status, note, velocity);
}

}
