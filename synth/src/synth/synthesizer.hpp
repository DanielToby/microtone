#pragma once

#include <common/midi_handle.hpp>
#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/wave_table.hpp>

#include <functional>
#include <memory>

namespace synth {

class Synthesizer {
public:
    Synthesizer(double sampleRate, const std::vector<WeightedWaveTable>& waveTables);
    Synthesizer(const Synthesizer&) = delete;
    Synthesizer& operator=(const Synthesizer&) = delete;
    Synthesizer(Synthesizer&&) noexcept;
    Synthesizer& operator=(Synthesizer&&) noexcept;
    ~Synthesizer();

    std::vector<WeightedWaveTable> waveTables() const;
    void setWaveTables(const std::vector<WeightedWaveTable>& tables);
    void setEnvelope(const Envelope& envelope);
    void setFilter(const Filter& filter);

    float nextSample(const common::midi::Keyboard& keyboard);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
