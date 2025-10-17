#pragma once

#include <synth/audio_buffer.hpp>
#include <synth/filter.hpp>
#include <synth/envelope.hpp>
#include <synth/wave_table.hpp>

#include <array>
#include <functional>
#include <memory>

namespace synth {

using OnOutputFn = std::function<void(const AudioBuffer&)>;

class Synthesizer {
public:
    Synthesizer(const std::vector<WeightedWaveTable>&, OnOutputFn);
    Synthesizer(const Synthesizer&) = delete;
    Synthesizer& operator=(const Synthesizer&) = delete;
    Synthesizer(Synthesizer&&) noexcept;
    Synthesizer& operator=(Synthesizer&&) noexcept;
    ~Synthesizer();

    void start();
    void stop();

    std::vector<WeightedWaveTable> weightedWaveTables() const;
    void setWaveTables(const std::vector<WeightedWaveTable>& tables);
    void setEnvelope(const Envelope& envelope);
    void setFilter(const Filter& filter);

    void addMidiData(int status, int note, int velocity);
    double sampleRate();

    std::vector<WeightedWaveTable>& getWaveTables() const;

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
