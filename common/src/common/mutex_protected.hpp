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

    MutexProtected& operator=(MutexProtected&& other) {
        if (this != &other) {
            _value = std::move(other._value);
        }
        return *this;
    }

    //! Blocks until T is obtained, then invokes `fn`.
    template <typename Fn>
    std::invoke_result_t<Fn, T&> get(Fn&& fn) {
        std::unique_lock lock(_mutex);
        return std::invoke(std::forward<Fn>(fn), _value);
    }

    //! Const overload
    template <typename Fn>
    std::invoke_result_t<Fn, const T&> get(Fn&& fn) const {
        std::unique_lock lock(_mutex);
        return std::invoke(std::forward<Fn>(fn), _value);
    }

    //! Tries to obtain `T` to invoke `fn`. Fn is not invoked if `T` is held by someone else.
    template <typename Fn>
    std::optional<std::invoke_result_t<Fn, T&>> ifAvailableThen(Fn&& fn) {
        if (_mutex.try_lock()) {
            auto unlock = std::unique_lock(_mutex, std::adopt_lock);
            return std::invoke(std::forward<Fn>(fn), _value);
        }
        return std::nullopt;
    }

private:
    T _value;
    mutable std::mutex _mutex;
};

}