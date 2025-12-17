#pragma once

#include <asciiboard/asciiboard.hpp>
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

constexpr auto chromaticScale = std::array{
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

}

//! Generates midi messages for demos.
class MidiGenerator {
public:
    MidiGenerator() = delete;
    explicit MidiGenerator(std::shared_ptr<common::midi::MidiHandle> midiHandle) :
        _midiHandle(std::move(midiHandle)) {}

    ~MidiGenerator() {
        this->stop();
    }

    void start() {
        _running = true;
        _thread = std::thread(&MidiGenerator::loop, this);
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
            const auto& scale = detail::chromaticScale;
            auto notePlaying = _notePlaying.load();
            auto index = _index.load();
            if (notePlaying) {
                _midiHandle->noteOff(scale[index].note);
                _notePlaying.store(false);
                _index.store((index + 1) % scale.size());
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } else {
                _midiHandle->noteOn(scale[index].note, scale[index].velocity);
                _notePlaying.store(true);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    std::shared_ptr<common::midi::MidiHandle> _midiHandle;
    std::atomic<bool> _running{false};
    std::thread _thread;

    std::atomic<std::size_t> _index = 0;
    std::atomic<bool> _notePlaying{false};
};

}
