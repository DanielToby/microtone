#pragma once

#include <array>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <optional>

#include "common/exception.hpp"

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

    virtual void initialize(const GPIOState& state) = 0;

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
    PushButton(PushButtonConfig config) : _config(std::move(config)) {}

    [[nodiscard]] bool isPinReserved(std::size_t pin) const override {
        return pin == _config.pin;
    }

    void initialize(const GPIOState& state) override {
        _isPressed = !state.isOn(_config.pin); //< Pressing connects pin to ground.
    }

    void update(const GPIOState& state) override {
        if (!_isPressed.has_value()) {
            throw common::MicrotoneException("Not initialized.");
        }

        const auto newIsPressed = !state.isOn(_config.pin);  //< Pressing connects pin to ground.
        if (*_isPressed && !newIsPressed) {
            std::invoke(_config.onReleased);
        } else if (!*_isPressed && newIsPressed) {
            std::invoke(_config.onPressed);
        }
        _isPressed = newIsPressed;
    }

private:
    std::optional<bool> _isPressed;
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

    void initialize(const GPIOState& state) override {
        _previousState = packState(_config, state);
    }

    void update(const GPIOState& state) override {
        if (!_previousState.has_value()) {
            throw common::MicrotoneException("Not initialized.");
        }

        auto currentState = packState(_config, state);

        auto currentTurn = getTurn(*_previousState, currentState);
        if (currentTurn == CCW) {
            _accumulatedTurn--;
        } else if (currentTurn == CW) {
            _accumulatedTurn++;
        }

        if (currentState == 0b00) {
            if (_accumulatedTurn >= 2) {
                std::invoke(_config.onCWTurn);
            } else if (_accumulatedTurn <= -2) {
                std::invoke(_config.onCCWTurn);
            }
            _accumulatedTurn = 0;
        }

        _previousState = currentState;
    }

private:
    [[nodiscard]] static int packState(const RotaryEncoderConfig& config, const GPIOState& state) {
        return static_cast<int>(state.isOn(config.CLK)) << 1 | static_cast<int>(state.isOn(config.DT));
    }

    enum Turn {
        None = 0,
        CW,
        CCW
    };

    //! This is a quadrature state machine.
    [[nodiscard]] static Turn getTurn(int previousState, int currentState) {
        constexpr std::array table = {
            None, CCW, CW, None,
            CW, None, None, CCW,
            CCW, None, None, CW,
            None, CW, CCW, None
        };

        const auto index = (previousState << 2) | currentState;
        return table[index];
    }

    std::optional<int> _previousState;
    int _accumulatedTurn = 0;
    RotaryEncoderConfig _config;
};

//! The hardware API supported by this library.
struct HardwareConfiguration {
    std::string chipName;
    std::string consumerName;
    std::vector<std::shared_ptr<I_GPIOComponent>> components;

    [[nodiscard]] bool isValid() const {
        for (std::size_t pin = 1; pin < GPIOState::maxPin; ++pin) {
            auto isReserved = [pin](const auto& component) {
                return component->isPinReserved(pin);
            };
            if (const auto numConsumers = std::ranges::count_if(components, isReserved); numConsumers > 1) {
                return false;
            }
        }
        return true;
    }
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
