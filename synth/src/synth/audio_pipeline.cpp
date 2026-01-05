#include "synth/audio_pipeline.hpp"

#include "common/exception.hpp"
#include "common/timer.hpp"

namespace synth {

namespace {
void logAudioBlockStatistics(const common::audio::FrameBlock& block, double blockDuration_us, std::chrono::microseconds computeDuration) {
    if (block.empty()) {
        return;
    }

    if (static_cast<double>(computeDuration.count()) > blockDuration_us) {
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

void AudioPipeline::processBlock() {
    // Get next input frame input device.
    auto nextBlockAndInputTime = common::timedInvoke([this] { return _source->getNextBlock(); });

    // This is necessary because capturing structured bindings is finicky.
    auto nextBlock = nextBlockAndInputTime.first;
    const auto inputDeviceTime = nextBlockAndInputTime.second;

    // Log synth generation time.
    const auto blockDuration_us = common::audio::getDuration_us(nextBlock.size(), _source->sampleRate());
    logAudioBlockStatistics(nextBlock, blockDuration_us, inputDeviceTime);

    // Apply effects.
    auto applyEffectsTime = common::timedInvoke([&] {
        for (const auto& effect : _effects) {
            effect->push(nextBlock);
            nextBlock = effect->getNextBlock();
        }
    });

    // Log effect time.
    logAudioBlockStatistics(nextBlock, blockDuration_us, applyEffectsTime);

    // Write to output device
    if (!_sink->push(nextBlock)) {
        // This is technically possible if someone else is writing to outputHandle.
        throw common::MicrotoneException("Processed block without room in sink.");
    }
}
}