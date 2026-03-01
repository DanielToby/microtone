#include <io/gpio_input.hpp>

#ifdef ENABLE_GPIO_CONTROL
#include <gpiod.hpp>
#endif

#include <atomic>
#include <thread>

#include "common/exception.hpp"
#include "common/log.hpp"

namespace io {

#ifdef ENABLE_GPIO_CONTROL

namespace {

struct LibGPIOManager {
    gpiod::chip chip;
    gpiod::line_request lineRequest;
};

//! Transforms the public facing config into a libgpiod config. Throws on failure!
[[nodiscard]] LibGPIOManager makeGPIOManager(const HardwareConfiguration& config) noexcept(false) {
    gpiod::chip chip(config.chipName.c_str());
    auto request = chip.prepare_request();

    gpiod::line_settings settings;
    settings.set_direction(gpiod::line::direction::INPUT);
    settings.set_edge_detection(gpiod::line::edge::BOTH);
    settings.set_bias(gpiod::line::bias::DISABLED);
    settings.set_debounce_period(std::chrono::milliseconds(10));

    for (std::size_t pin = 1; pin < GPIOState::maxPin; ++pin) {
        for (const auto& component : config.components) {
            if (component->isPinReserved(pin)) {
                request.add_line_settings(pin, settings);
            }
        }
    }

    request.set_consumer(config.consumerName);
    return {std::move(chip), request.do_request()};
}

[[nodiscard]] GPIOState getInitialState(LibGPIOManager& manager, const HardwareConfiguration& config) {
    GPIOState state;
    for (std::size_t pin = 1; pin < GPIOState::maxPin; ++pin) {
        for (const auto& component : config.components) {
            if (component->isPinReserved(pin)) {
                const bool isOn = manager.lineRequest.get_value(pin) == gpiod::line::value::ACTIVE;
                state.setPin(pin, isOn);
            }
        }
    }
    return state;
}

}

class GPIOInput::impl {
public:
    explicit impl(const HardwareConfiguration& hardwareConfig) :
        _hardwareConfig(hardwareConfig),
        _gpioManager(makeGPIOManager(_hardwareConfig)),
        _gpioState(getInitialState(_gpioManager, _hardwareConfig)) {
        if (!hardwareConfig.isValid()) {
            throw common::MicrotoneException("Invalid GPIO configuration.");
        }

        for (const auto& component : hardwareConfig.components) {
            component->initialize(_gpioState);
        }
    }

    void start() {
        _running = true;
        _thread = std::thread(&impl::processLoop, this);
        M_INFO("Opened GPIO input stream.");
    }

    void stop() {
        _running = false;
        if (_thread.joinable()) {
            _thread.join();
        }
        M_INFO("Closed GPIO input stream.");
    }

    ~impl() {
        stop();
    }

private:
    void processLoop() {
        while (_running) {
            if (_gpioManager.lineRequest.wait_edge_events(std::chrono::seconds(1))) {
                _gpioManager.lineRequest.read_edge_events(_eventBuffer);

                for (const auto& edgeEvent : _eventBuffer) {
                    const auto pin = edgeEvent.line_offset();
                    const auto value = _gpioManager.lineRequest.get_value(pin);
                    const auto isOn = (value == gpiod::line::value::ACTIVE);
                    _gpioState.setPin(pin, isOn);

                    for (const auto& component : _hardwareConfig.components) {
                        if (component->isPinReserved(pin)) {
                            component->update(_gpioState);
                        }
                    }
                }
            }
        }
    }

    HardwareConfiguration _hardwareConfig;
    LibGPIOManager _gpioManager;
    GPIOState _gpioState;
    gpiod::edge_event_buffer _eventBuffer;

    std::atomic<bool> _running{false};
    std::thread _thread;
};

#else

class GPIOInput::impl {
public:
    explicit impl([[maybe_unused]] const HardwareConfiguration& hardwareConfig) {}
    static void start() {
        M_INFO("GPIO input is disabled. Configure CMake with -DENABLE_GPIO_CONTROL=ON to enable it.");
    }
    static void stop() {}
};

#endif

GPIOInput::GPIOInput(const HardwareConfiguration& config) :
    _impl{std::make_unique<impl>(config)} {
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

void GPIOInput::start() const {
    _impl->start();
}

void GPIOInput::stop() const {
    _impl->stop();
}

}
