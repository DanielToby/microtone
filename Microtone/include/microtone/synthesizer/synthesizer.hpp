#pragma once

#include <microtone/microtone_platform.hpp>
#include <microtone/synthesizer/audio_buffer.hpp>
#include <microtone/synthesizer/envelope.hpp>
#include <microtone/synthesizer/filter.hpp>
#include <microtone/synthesizer/weighted_wavetable.hpp>

#include <array>
#include <functional>
#include <memory>

namespace microtone {

using OnOutputFn = std::function<void(const AudioBuffer&)>;

class Synthesizer {
public:
    explicit Synthesizer(const std::vector<WeightedWaveTable>&, OnOutputFn);
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
