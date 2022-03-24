#pragma once

#include <memory>

namespace microtone {

class Synthesizer {
public:
    explicit Synthesizer();
    Synthesizer(const Synthesizer&) = delete;
    Synthesizer& operator=(const Synthesizer&) = delete;
    Synthesizer(Synthesizer&&) noexcept;
    Synthesizer& operator=(Synthesizer&&) noexcept;
    ~Synthesizer();

    void addNoteData(int note, int velocity, double timeStamp);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
