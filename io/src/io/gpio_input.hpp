#pragma once

#include <memory>

namespace io {

//! This is the portaudio wrapper.
class GPIOInput {
public:
    GPIOInput();
    GPIOInput(const GPIOInput&) = delete;
    GPIOInput& operator=(const GPIOInput&) = delete;
    GPIOInput(GPIOInput&&) noexcept;
    GPIOInput& operator=(GPIOInput&&) noexcept;
    ~GPIOInput();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
