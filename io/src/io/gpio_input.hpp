#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>

namespace io {

//! This library supports a 1-indexed 40 pin GPIO.
class GPIOState {
public:
    static constexpr std::size_t maxPin = 41;

    GPIOState() = default;
    GPIOState(const GPIOState&) = default;

    [[nodiscard]] bool isOn(std::size_t pin) const {
        return _zeroIndexedState[pin - 1];
    }

    void setPin(std::size_t pin, bool value) {
        _zeroIndexedState[pin - 1] = value;
    }

private:
    std::array<bool, maxPin -1> _zeroIndexedState;
};

//! Occupies some compile-time known number of pins.
//! If any of those pins are affected when events are being processed, "processEvents" is called with the latest state.
class I_GPIOComponent {
public:
    virtual ~I_GPIOComponent() = default;

    //! Pins are 1-indexed.
    [[nodiscard]] virtual bool isPinReserved(std::size_t pin) const = 0;

    //! This function is called when a reserved pin is changed.
    virtual void update(const GPIOState& state) = 0;
};

using OnEventFn = std::function<void()>;

struct PushButtonConfig {
    std::size_t pin;
    OnEventFn onPressed;
    OnEventFn onReleased;
};

class PushButton : public I_GPIOComponent {
public:
    PushButton(PushButtonConfig config) : _previousState(), _config(std::move(config)) {}

    [[nodiscard]] bool isPinReserved(std::size_t pin) const override {
        return pin == _config.pin;
    }

    void update(const GPIOState& state) override {
        if (_previousState.isOn(_config.pin) && !state.isOn(_config.pin)) {
            std::invoke(_config.onPressed);
        } else if (!_previousState.isOn(_config.pin) && state.isOn(_config.pin)) {
            std::invoke(_config.onReleased);
        }
        _previousState = state;
    }

private:
    GPIOState _previousState;
    PushButtonConfig _config;
};

struct RotaryEncoderConfig {
    std::size_t CLK;
    std::size_t DT;
    OnEventFn onCWTurn;
    OnEventFn onCCWTurn;
};

class RotaryEncoder : public I_GPIOComponent {
public:
    RotaryEncoder(const RotaryEncoderConfig& config) :
        _previousState(packState(config, GPIOState{})),
        _config(config) {}

    [[nodiscard]] bool isPinReserved(std::size_t pin) const override {
        return pin == _config.CLK || pin == _config.DT;
    }

    //! Based on a quadrature state machine.
    void update(const GPIOState& state) override {
        // No rotation = 0, CW = +1, CCW = -1;
        constexpr std::array<int8_t, 16> table = {
            0,  +1, -1,  0,
           -1,   0,  0, +1,
           +1,   0,  0, -1,
            0,  -1, +1,  0
        };

        auto currentState = packState(_config, state);

        auto index = (_previousState << 2) | currentState;
        auto value = table[index];
        if (value == 1) {
            std::invoke(_config.onCWTurn);
        } else if (value == -1) {
            std::invoke(_config.onCCWTurn);
        }

        _previousState = currentState;
    }

private:
    [[nodiscard]] static int packState(const RotaryEncoderConfig& config, const GPIOState& state) {
        return static_cast<int>(state.isOn(config.CLK)) << 1 | static_cast<int>(config.DT);
    }

    int _previousState;
    RotaryEncoderConfig _config;
};

//! The hardware API supported by this library.
struct HardwareConfiguration {
    std::string chipName;
    std::string consumerName;
    std::vector<std::shared_ptr<I_GPIOComponent>> components;
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

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
