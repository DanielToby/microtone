#pragma once

#include <microtone/microtone_platform.hpp>
#include <microtone/envelope.hpp>
#include <microtone/filter.hpp>

#include <array>
#include <functional>
#include <memory>

namespace microtone {

const int FRAMES_PER_BUFFER = 512;
using OnOutputFn = std::function<void(const std::array<float, FRAMES_PER_BUFFER>&)>;

class Synthesizer {
public:
    explicit Synthesizer(OnOutputFn);
    Synthesizer(const Synthesizer&) = delete;
    Synthesizer& operator=(const Synthesizer&) = delete;
    Synthesizer(Synthesizer&&) noexcept;
    Synthesizer& operator=(Synthesizer&&) noexcept;
    ~Synthesizer();

    void setEnvelope(const Envelope& envelope);
    void setFilter(const Filter& filter);
    void addMidiData(int status, int note, int velocity);
    double sampleRate();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
