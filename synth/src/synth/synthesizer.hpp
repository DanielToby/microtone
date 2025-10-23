#pragma once

#include <synth/audio_buffer.hpp>
#include <synth/controller.hpp>
#include <synth/envelope.hpp>
#include <synth/filter.hpp>
#include <synth/wave_table.hpp>

#include <array>
#include <functional>
#include <memory>

namespace synth {

using OnOutputFn = std::function<void(const AudioBuffer&)>;

class Synthesizer : public I_Controller {
public:
    Synthesizer(double sampleRate, const std::vector<WeightedWaveTable>& waveTables);
    Synthesizer(const Synthesizer&) = delete;
    Synthesizer& operator=(const Synthesizer&) = delete;
    Synthesizer(Synthesizer&&) noexcept;
    Synthesizer& operator=(Synthesizer&&) noexcept;
    ~Synthesizer();

    std::vector<WeightedWaveTable> weightedWaveTables() const;
    void setWaveTables(const std::vector<WeightedWaveTable>& tables);
    void setEnvelope(const Envelope& envelope);
    void setFilter(const Filter& filter);

    float nextSample();

    void noteOn(int note, int velocity) override;
    void noteOff(int note) override;
    void sustainOn() override;
    void sustainOff() override;

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
