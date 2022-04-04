#pragma once

#include <microtone/microtone_platform.hpp>

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

    void addNoteData(int status, int note, int velocity);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
