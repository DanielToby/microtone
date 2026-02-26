#include <io/gpio_input.hpp>

#ifdef ENABLE_GPIO_CONTROL

#include <gpiod/version.h>

#if GPIOD_VERSION_MAJOR < 2
#error "libgpiod >= 2.0 is required"
#endif

#endif

namespace io {

class GPIOInput::impl {
public:
    impl() {
    }

    ~impl() {
    }
};

GPIOInput::GPIOInput() :
    _impl{std::make_unique<impl>()} {
}

GPIOInput::GPIOInput(GPIOInput&& other) noexcept :
    _impl{std::move(other._impl)} {
}

GPIOInput& GPIOInput::operator=(GPIOInput&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

GPIOInput::~GPIOInput() = default;

}
