#pragma once

#include <asciiboard/asciiboard.hpp>
#include <common/log.hpp>
#include <common/midi_handle.hpp>
#include <common/ring_buffer.hpp>

#include <memory>
#include <thread>

namespace asciiboard::demo {
namespace detail {

//! A demo note.
struct Note {
    std::size_t note;
    int velocity;
};

constexpr auto chromaticScaleVaryingVelocity = std::array{
    Note{60, 40}, // Middle C.
    Note{61, 45},
    Note{62, 50},
    Note{63, 55},
    Note{64, 60},
    Note{65, 65},
    Note{66, 70},
    Note{67, 75},
    Note{68, 80},
    Note{69, 85},
    Note{70, 90},
    Note{71, 95},
    Note{72, 100},
    Note{71, 95},
    Note{70, 90},
    Note{69, 85},
    Note{68, 80},
    Note{67, 75},
    Note{66, 70},
    Note{65, 65},
    Note{64, 60},
    Note{63, 55},
    Note{62, 50},
    Note{61, 45}
};

constexpr auto chromaticScaleFixedVelocity = std::array{
    Note{60, 50}, // Middle C.
    Note{61, 50},
    Note{62, 50},
    Note{63, 50},
    Note{64, 50},
    Note{65, 50},
    Note{66, 50},
    Note{67, 50},
    Note{68, 50},
    Note{69, 50},
    Note{70, 50},
    Note{71, 50},
    Note{72, 50},
    Note{71, 50},
    Note{70, 50},
    Note{69, 50},
    Note{68, 50},
    Note{67, 50},
    Note{66, 50},
    Note{65, 50},
    Note{64, 50},
    Note{63, 50},
    Note{62, 50},
    Note{61, 50}
};
}

enum class Sequence {
    ChromaticScale = 0,
    CMaj7
};

enum class UseFixedVelocity : bool { no, yes };

[[nodiscard]] inline const auto& getScale(UseFixedVelocity useFixedVelocity) {
    if (static_cast<bool>(useFixedVelocity)) {
        return detail::chromaticScaleFixedVelocity;
    }
    return detail::chromaticScaleVaryingVelocity;
}

struct MidiGeneratorOptions {
    std::chrono::milliseconds noteOnTime;
    std::chrono::milliseconds noteOffTime;
    Sequence sequence;
    UseFixedVelocity useFixedVelocity;
};

//! Generates midi messages for demos.
class MidiGenerator {
public:
    MidiGenerator() = delete;
    explicit MidiGenerator(std::shared_ptr<common::midi::TwoReaderMidiHandle> midiHandle, MidiGeneratorOptions options) :
        _midiHandle(std::move(midiHandle)),
        _options(options) {}

    ~MidiGenerator() {
        this->stop();
    }

    void start() {
        _running = true;
        _thread = std::thread(&MidiGenerator::loop, this);
        M_INFO("Started midi generator.");
    }

    void stop() {
        _running = false;
        if (_thread.joinable())
            _thread.join();
    }

private:
    void loop() {
        _running.store(true);
        while (_running.load() == true) {
            const auto notePlaying = _notePlaying.load();
            const auto index = _index.load();
            const auto& scale = getScale(_options.useFixedVelocity);

            auto alternate = [&, this](auto onNoteOn, auto onNoteOff) {
                if (notePlaying) {
                    std::invoke(onNoteOff);
                    _notePlaying.store(false);
                    _index.store((index + 1) % scale.size());
                    std::this_thread::sleep_for(_options.noteOffTime);
                } else {
                    std::invoke(onNoteOn);
                    _notePlaying.store(true);
                    std::this_thread::sleep_for(_options.noteOnTime);
                }
            };

            switch (_options.sequence) {
            case Sequence::ChromaticScale: {
                alternate([&, this] {
                    _midiHandle->noteOn(scale[index].note, scale[index].velocity); },
                          [&, this] {
                              _midiHandle->noteOff(scale[index].note);
                          });
                break;
            }
            case Sequence::CMaj7: {
                alternate([&, this] {
                    _midiHandle->noteOn(scale[index].note, scale[index].velocity);
                    _midiHandle->noteOn(scale[index].note + 4, scale[index].velocity);
                    _midiHandle->noteOn(scale[index].note + 7, scale[index].velocity);
                    _midiHandle->noteOn(scale[index].note + 11, scale[index].velocity); },
                          [&, this] {
                              _midiHandle->noteOff(scale[index].note);
                              _midiHandle->noteOff(scale[index].note + 4);
                              _midiHandle->noteOff(scale[index].note + 7);
                              _midiHandle->noteOff(scale[index].note + 11);
                          });
                break;
            }
            default:
                throw common::MicrotoneException("Unsupported generated midi sequence.");
            }
        }
    }

    std::shared_ptr<common::midi::TwoReaderMidiHandle> _midiHandle;
    MidiGeneratorOptions _options;

    std::atomic<bool> _running{false};
    std::thread _thread;
    std::atomic<std::size_t> _index = 0;
    std::atomic<bool> _notePlaying{false};
};

}
