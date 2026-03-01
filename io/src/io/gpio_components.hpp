#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <variant>

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
    std::array<bool, maxPin - 1> _zeroIndexedState;
};

using OnEventFn = std::function<void()>;

struct PushButtonConfig {
    std::size_t pin;
    OnEventFn onPressed;
    OnEventFn onReleased;

    [[nodiscard]] bool isPinReserved(std::size_t p) const {
        return p == pin;
    }
};

struct RotaryEncoderConfig {
    std::size_t CLK;
    std::size_t DT;
    OnEventFn onCWTurn;
    OnEventFn onCCWTurn;

    [[nodiscard]] bool isPinReserved(std::size_t p) const {
        return p == CLK || p == DT;
    }
};

using ComponentConfig = std::variant<PushButtonConfig, RotaryEncoderConfig>;

namespace detail {

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

class PushButton : public I_GPIOComponent {
public:
    PushButton(PushButtonConfig config, const GPIOState& state) :
        _config(std::move(config)),
        _isPressed(getIsPressed(state)) {}

    [[nodiscard]] bool isPinReserved(std::size_t pin) const override {
        return _config.isPinReserved(pin);
    }

    void update(const GPIOState& state) override {
        const auto newIsPressed = getIsPressed(state);
        if (_isPressed && !newIsPressed) {
            std::invoke(_config.onReleased);
        } else if (!_isPressed && newIsPressed) {
            std::invoke(_config.onPressed);
        }
        _isPressed = newIsPressed;
    }

private:
    [[nodiscard]] bool getIsPressed(const GPIOState& state) const {
        return !state.isOn(_config.pin);//< Pressing connects pin to ground.
    }

    PushButtonConfig _config;
    bool _isPressed;
};

class RotaryEncoder : public I_GPIOComponent {
public:
    RotaryEncoder(RotaryEncoderConfig config, const GPIOState& state) :
        _config(std::move(config)),
        _previousState(packState(state)) {}

    [[nodiscard]] bool isPinReserved(std::size_t pin) const override {
        return _config.isPinReserved(pin);
    }

    void update(const GPIOState& state) override {
        auto currentState = packState(state);

        auto currentTurn = getTurn(_previousState, currentState);
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
    [[nodiscard]] int packState(const GPIOState& state) const {
        return static_cast<int>(state.isOn(_config.CLK)) << 1 | static_cast<int>(state.isOn(_config.DT));
    }

    enum Turn {
        None = 0,
        CW,
        CCW
    };

    //! This is a quadrature state machine.
    [[nodiscard]] static Turn getTurn(int previousState, int currentState) {
        constexpr std::array table = {
            None, CCW, CW, None, CW, None, None, CCW, CCW, None, None, CW, None, CW, CCW, None};

        const auto index = (previousState << 2) | currentState;
        return table[index];
    }

    RotaryEncoderConfig _config;
    int _previousState;
    int _accumulatedTurn{0};
};

template <typename ConfigT>
[[nodiscard]] bool isPinReserved(const ConfigT& config, std::size_t pin) {
    return config.isPinReserved(pin);
}

template <typename ConfigT>
[[nodiscard]] std::shared_ptr<I_GPIOComponent> makeComponent(ConfigT&& config, const GPIOState& state);

template <>
[[nodiscard]] inline std::shared_ptr<I_GPIOComponent> makeComponent(PushButtonConfig&& config, const GPIOState& state) {
    return std::make_shared<PushButton>(std::move(config), state);
}

template <>
[[nodiscard]] inline std::shared_ptr<I_GPIOComponent> makeComponent(RotaryEncoderConfig&& config, const GPIOState& state) {
    return std::make_shared<RotaryEncoder>(std::move(config), state);
}

[[nodiscard]] inline std::vector<std::shared_ptr<I_GPIOComponent>> makeComponents(const std::vector<ComponentConfig>& configs, const GPIOState& state) {
    std::vector<std::shared_ptr<I_GPIOComponent>> result;
    const auto make = [&state]<typename T0>(T0&& cfg) {
        return makeComponent(std::forward<T0>(cfg), state);
    };
    for (auto config : configs) {
        result.push_back(std::visit(make, std::move(config)));
    }

    return result;
}

}

}