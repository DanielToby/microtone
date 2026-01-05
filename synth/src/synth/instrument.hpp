#pragma once

#include <common/timer.hpp>

#include <memory>
#include <thread>

namespace synth {

namespace detail {

inline void logAudioBlockStatistics(const common::audio::FrameBlock& block, double blockDuration_us, double computeDuration_us) {
    if (block.empty()) {
        return;
    }

    if (computeDuration_us > blockDuration_us) {
        M_WARN("Compute took longer than block duration.");
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

//! Runs the input in a dedicated process that executes only if the output has space.
//! Forwards midi changes to inputs as soon as they're available.
class Instrument {
public:
    Instrument() = delete;
    Instrument(std::shared_ptr<I_SourceNode> source,
               std::shared_ptr<const common::midi::MidiHandle> midiHandle,
               std::shared_ptr<I_SinkNode> sink) :
        _source(std::move(source)),
        _midiHandle(std::move(midiHandle)),
        _sink(std::move(sink)) {}

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
            if (_midiHandle->hasChanges()) {
                _source->respondToKeyboardChanges(_midiHandle->read());
            }
            if (!_sink->isFull()) {
                auto [nextBlock, duration] = common::timedInvoke([this]() { return _source->getNextBlock(); });
                detail::logAudioBlockStatistics(
                    nextBlock,
                    common::audio::getDuration_us(nextBlock.size(), _source->sampleRate()),
                    static_cast<double>(duration.count()));
                if (!_sink->push(nextBlock)) {
                    // This is technically possible if someone else is writing to outputHandle.
                    throw common::MicrotoneException("Incremented synth but discarded the result.");
                }
            }
        }
    }

    std::shared_ptr<I_SourceNode> _source;
    std::shared_ptr<const common::midi::MidiHandle> _midiHandle;
    std::shared_ptr<I_SinkNode> _sink;

    std::atomic<bool> _running{false};
    std::thread _thread;
};

}
