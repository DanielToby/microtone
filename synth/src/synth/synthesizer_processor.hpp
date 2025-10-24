#pragma once

#include <common/midi_handle.hpp>
#include <common/ring_buffer.hpp>
#include <synth/synthesizer.hpp>

#include <thread>

namespace synth {

//! The thread safe handles that the synth processor needs to operate safely.
struct SynthesizerProcessorHandles {
    Synthesizer& synthesizer;

    const common::midi::MidiHandle& midiHandle;
    common::audio::RingBuffer<>& outputHandle;
};

//! Runs the Synthesizer in a dedicated process that executes only if the buffer has room.
class SynthesizerProcessor {
public:
    SynthesizerProcessor() = delete;
    explicit SynthesizerProcessor(const SynthesizerProcessorHandles& handles) :
        _handles(handles) {}

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
            const auto numSamples = _handles.outputHandle.availableSpace();
            for (auto i = 0; i < numSamples; ++i) {
                const auto sample = _handles.synthesizer.nextSample(_handles.midiHandle.getKeyboardState());
                _handles.outputHandle.push(sample);
            }
        }
    }

    SynthesizerProcessorHandles _handles;

    std::atomic<bool> _running{false};
    std::thread _thread;
};

}
