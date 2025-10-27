#pragma once

#include <common/midi_handle.hpp>
#include <common/ring_buffer.hpp>
#include <synth/synthesizer.hpp>

#include <memory>
#include <thread>

namespace synth {

//! Runs the Synthesizer in a dedicated process that executes only if the buffer has room.
class SynthesizerProcessor {
public:
    SynthesizerProcessor() = delete;
    SynthesizerProcessor(std::shared_ptr<Synthesizer> synthesizer,
                         std::shared_ptr<const common::midi::MidiHandle> midiHandle,
                         std::shared_ptr<common::audio::RingBuffer<>> outputHandle) :
        _synthesizer(std::move(synthesizer)),
        _midiHandle(std::move(midiHandle)),
        _outputHandle(std::move(outputHandle)) {}

    void start() {
        _running = true;
        _thread = std::thread(&SynthesizerProcessor::processLoop, this);
    }

    void stop() {
        _running = false;
        if (_thread.joinable())
            _thread.join();
    }

private:
    void processLoop() {
        while (_running) {
            // More space might become available while we run. Let the next loop get that.
            const auto numSamples = _outputHandle->availableSpace();
            for (auto i = 0; i < numSamples; ++i) {
                const auto sample = _synthesizer->nextSample(_midiHandle->getKeyboardState());
                _outputHandle->push(sample.value_or(0.0f)); // TODO: Not this.
            }
        }
    }

    std::shared_ptr<Synthesizer> _synthesizer;
    std::shared_ptr<const common::midi::MidiHandle> _midiHandle;
    std::shared_ptr<common::audio::RingBuffer<>> _outputHandle;

    std::atomic<bool> _running{false};
    std::thread _thread;
};

}
