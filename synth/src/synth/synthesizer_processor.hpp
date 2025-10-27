#pragma once

#include <common/midi_handle.hpp>
#include <common/ring_buffer.hpp>
#include <synth/synthesizer.hpp>

#include <thread>

namespace synth {

//! The thread safe handles that the synth processor needs to operate safely.
struct SynthesizerProcessorHandles {

};

//! Runs the Synthesizer in a dedicated process that executes only if the buffer has room.
class SynthesizerProcessor {
public:
    SynthesizerProcessor() = delete;
    SynthesizerProcessor(Synthesizer& synthesizer,
                         const common::midi::MidiHandle& midiHandle,
                         common::audio::RingBuffer<>& outputHandle) :
        _synthesizer(synthesizer),
        _midiHandle(midiHandle),
        _outputHandle(outputHandle) {}

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
            const auto numSamples = _outputHandle.availableSpace();
            for (auto i = 0; i < numSamples; ++i) {
                const auto sample = _synthesizer.nextSample(_midiHandle.getKeyboardState());
                _outputHandle.push(sample.value_or(0.0f)); // TODO: Not this.
            }
        }
    }

    Synthesizer& _synthesizer;
    const common::midi::MidiHandle& _midiHandle;
    common::audio::RingBuffer<>& _outputHandle;

    std::atomic<bool> _running{false};
    std::thread _thread;
};

}
