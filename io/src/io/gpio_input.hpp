#pragma once

#include <functional>
#include <memory>
#include <string>

#include "io/gpio_components.hpp"

namespace io {

//! The hardware API supported by this library.
struct HardwareConfiguration {
    std::string chipName;
    std::string consumerName;
    std::vector<ComponentConfig> componentConfigs;

    [[nodiscard]] bool isValid() const;
};

//! This is the portaudio wrapper.
class GPIOInput {
public:
    explicit GPIOInput(const HardwareConfiguration& config);
    GPIOInput(const GPIOInput&) = delete;
    GPIOInput& operator=(const GPIOInput&) = delete;
    GPIOInput(GPIOInput&&) noexcept;
    GPIOInput& operator=(GPIOInput&&) noexcept;
    ~GPIOInput();

    void start() const;
    void stop() const;

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
