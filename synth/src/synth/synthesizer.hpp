#pragma once

#include <common/ring_buffer.hpp>
#include <common/midi_handle.hpp>
#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/wave_table.hpp>

#include <functional>
#include <memory>
#include <optional>

namespace synth {

class Synthesizer {
public:
    Synthesizer(double sampleRate, const std::vector<WeightedWaveTable>& waveTables);
    Synthesizer(const Synthesizer&) = delete;
    Synthesizer& operator=(const Synthesizer&) = delete;
    Synthesizer(Synthesizer&&) noexcept;
    Synthesizer& operator=(Synthesizer&&) noexcept;
    ~Synthesizer();

    //! Thread safe configurable state.
    //! TODO: No raw synchronization primitives. Move this to a thread_safe_accessible<T>.
    std::vector<WeightedWaveTable> waveTables() const;
    void setWaveTables(const std::vector<WeightedWaveTable>& tables);
    void setEnvelope(const Envelope& envelope);
    void setFilter(const Filter& filter);

    //! Respond to changes in the keyboard (trigger voices on or off).
    void respondToKeyboardChanges(const common::midi::Keyboard& keyboard);

    //! Increments counters in envelopes and everything. It's probably not a good idea to throw away the result!
    [[nodiscard]] common::audio::FrameBlock getNextBlock();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
