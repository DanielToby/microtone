#pragma once

#include <asciiboard/asciiboard.hpp>
#include <common/midi_handle.hpp>
#include <synth/synthesizer.hpp>

#include <memory>
#include <thread>

namespace asciiboard {

//! FTXUI is supposed to be event driven, but I don't like passing lambdas everywhere.
class RenderLoop {
public:
    RenderLoop() = delete;
    RenderLoop(std::shared_ptr<Asciiboard> ui,
               std::shared_ptr<const common::midi::TwoReaderMidiHandle> midiHandle,
               std::shared_ptr<const synth::Synthesizer> synthHandle) :
        _ui(std::move(ui)),
        _synthHandle(std::move(synthHandle)),
        _midiHandle(std::move(midiHandle)),
        _midiReaderId(_midiHandle->registerReader()) {}

    ~RenderLoop() {
        this->stop();
    }

    void start() {
        _running = true;
        _thread = std::thread(&RenderLoop::renderLoop, this);
    }

    void stop() {
        _running = false;
        if (_thread.joinable())
            _thread.join();
    }

private:
    void renderLoop() {
        while (_running) {
            if (const auto& lastAudioBlock = _synthHandle->getLastBlock()) {
                _ui->addOutputData(*lastAudioBlock);
            }
            if (!_midiReaderId) {
                throw common::MicrotoneException("Uninitialized (no midi reader ID).");
            }
            if (_midiHandle->hasChanges(*_midiReaderId)) {
                _ui->updateMidiKeyboard(_midiHandle->read(*_midiReaderId));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    }
    std::shared_ptr<Asciiboard> _ui;
    std::shared_ptr<const synth::Synthesizer> _synthHandle;
    std::shared_ptr<const common::midi::TwoReaderMidiHandle> _midiHandle;
    std::optional<std::size_t> _midiReaderId;

    std::atomic<bool> _running{false};
    std::thread _thread;
};

}
