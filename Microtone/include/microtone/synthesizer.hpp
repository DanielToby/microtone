#pragma once

#include <microtone/microtone_platform.hpp>
#include <microtone/envelope.hpp>
#include <microtone/filter.hpp>

#include <atomic>
#include <functional>
#include <memory>


using OnOutputFn = std::function<void(const std::vector<float>&)>;

namespace microtone {

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

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
