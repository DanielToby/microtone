#pragma once

#include <memory>
#include <thread>

namespace synth {

//! Runs the input in a dedicated process that executes only if the output has space.
//! Forwards midi changes to inputs as soon as they're available.
class Instrument {
public:
    Instrument() = delete;
    Instrument(std::shared_ptr<const common::midi::TwoReaderMidiHandle> midiHandle, AudioPipeline&& pipeline) :
        _pipeline(std::move(pipeline)),
        _midiHandle(std::move(midiHandle)),
        _midiReaderId(_midiHandle->registerReader()) {}

    ~Instrument() {
        this->stop();
    }

    void start() {
        _running = true;
        _thread = std::thread(&Instrument::processLoop, this);
    }

    void stop() {
        _running = false;
        if (_thread.joinable())
            _thread.join();
    }

private:
    void processLoop() {
        while (_running) {
            if (!_midiReaderId) {
                throw common::MicrotoneException("Uninitialized (no midi reader ID).");
            }
            if (_midiHandle->hasChanges(*_midiReaderId)) {
                _pipeline.getSource().respondToKeyboardChanges(_midiHandle->read(*_midiReaderId));
            }
            if (_pipeline.shouldProcessBlock()) {
                _pipeline.processBlock();
            }
        }
    }

    AudioPipeline _pipeline;
    std::shared_ptr<const common::midi::TwoReaderMidiHandle> _midiHandle;
    std::optional<std::size_t> _midiReaderId;

    std::atomic<bool> _running{false};
    std::thread _thread;
};

}
