#pragma once

#include <functional>
#include <optional>
#include <mutex>

namespace common {

//! https://awesomekling.github.io/MutexProtected-A-C++-Pattern-for-Easier-Concurrency/
template <class T>
class MutexProtected {
public:
    MutexProtected() = default;
    explicit MutexProtected(const T& value) : _value(value) {}

    MutexProtected& operator=(MutexProtected&& other) noexcept {
        if (this != &other) {
            _value = std::move(other._value);
        }
        return *this;
    }

    //! Blocks to obtain a copy of T.
    T read() const {
        auto lock = std::unique_lock(_mutex);
        return _value;
    }

    //! Blocks to update T.
    void write(const std::function<void(T&)>& mutate) {
        auto lock = std::unique_lock(_mutex);
        std::invoke(mutate, _value);
    }

    //! Tries to obtain `T` to invoke `fn`. Fn is not invoked if `T` is held by someone else.
    std::optional<T> readIfAvailable() const {
        if (_mutex.try_lock()) {
            auto unlock = std::unique_lock(_mutex, std::adopt_lock);
            return _value;
        }
        return std::nullopt;
    }

private:
    T _value;
    mutable std::mutex _mutex;
};

}