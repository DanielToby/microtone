#pragma once

#include "audio_pipeline.hpp"

#include <common/midi_handle.hpp>
#include <common/ring_buffer.hpp>
#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/wave_table.hpp>

#include <functional>
#include <memory>
#include <optional>

namespace synth {

//! The synth has a fixed number of wavetables.
using TripleWaveTableT = WeightedWaveTables<3>;
using TripleWeightsT = std::array<float, 3>;

class Synthesizer : public I_SourceNode {
public:
    Synthesizer(double sampleRate, const TripleWaveTableT& waveTables, float gain, const ADSR& adsr, float lfoFrequencyHz, float lfoGain);
    Synthesizer(const Synthesizer&) = delete;
    Synthesizer& operator=(const Synthesizer&) = delete;
    Synthesizer(Synthesizer&&) noexcept;
    Synthesizer& operator=(Synthesizer&&) noexcept;
    ~Synthesizer();

    void setOscillatorWeights(const TripleWeightsT& tables);
    void setAdsr(const ADSR& adsr);
    void setFilter(const Filter& filter);
    void setGain(float gain);
    void setLfoFrequency(float frequencyHz);
    void setLfoGain(float gain);

    //! Respond to changes in the keyboard (trigger voices on or off).
    void respondToKeyboardChanges(const common::midi::Keyboard& keyboard) override;

    //! Increments counters in envelopes and everything. It's probably not a good idea to throw away the result!
    [[nodiscard]] common::audio::FrameBlock getNextBlock() override;

    //! The last block returned by `getNextBlock`, for bookkeeping.
    [[nodiscard]] const std::optional<common::audio::FrameBlock>&  getLastBlock() const;

    [[nodiscard]] double sampleRate() const override;

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
