#pragma once

#include <common/midi_handle.hpp>
#include <common/ring_buffer.hpp>
#include <synth/synthesizer.hpp>

#include <memory>
#include <thread>

namespace synth {

namespace detail {

inline void logAudioBlockStatistics(const common::audio::FrameBlock& block) {
    if (block.empty()) {
        return;
    }

    for (const auto& sample : block) {
        if (!std::isfinite(sample)) {
            M_WARN("Sample is NaN.");
        };

        if (sample > 1.0f) {
            M_WARN(fmt::format("Sample higher than 1.0: {}", sample));
        } else if (sample < -1.0f) {
            M_WARN(fmt::format("Sample less than -1.0: {}", sample));
        }
    }
}

}

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

    ~SynthesizerProcessor() {
        this->stop();
    }

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
            _synthesizer->respondToKeyboardChanges(_midiHandle->getKeyboardState());
            if (!_outputHandle->isFull()) {
                auto nextBlock = _synthesizer->getNextBlock();
                detail::logAudioBlockStatistics(nextBlock);
                if (!_outputHandle->push(nextBlock)) {
                    // This is technically possible if someone else is writing to outputHandle.
                    throw common::MicrotoneException("Incremented synth but discarded the result.");
                }
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
