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
                         std::shared_ptr<const common::midi::MidiHandle> midiHandle,
                         std::shared_ptr<const synth::Synthesizer> synthHandle) :
        _ui(std::move(ui)),
        _midiHandle(std::move(midiHandle)),
        _synthHandle(std::move(synthHandle)) {}

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
            _ui->updateMidiKeyboard(_midiHandle->read());
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    }
    std::shared_ptr<Asciiboard> _ui;
    std::shared_ptr<const common::midi::MidiHandle> _midiHandle;
    std::shared_ptr<const synth::Synthesizer> _synthHandle;

    std::atomic<bool> _running{false};
    std::thread _thread;
};

}
