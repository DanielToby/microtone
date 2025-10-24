#pragma once

#include <atomic>
#include <array>
#include <cstddef>
#include <optional>

namespace common::audio {

//! Provides thread-safe access to a buffer of Ts.
//! This class could be improved using batch operations to reduce atomic operations.

template <typename T = float, size_t N = 512>
class RingBuffer {
public:
    using value_type = T;
    static constexpr std::size_t capacity = N;

    RingBuffer() = default;

    //! Returns true if an item was pushed.
    bool push(const T& item) {
        size_t nextHead = (_head + 1) % N;
        if (nextHead == _tail.load(std::memory_order_acquire)) {
            return false; // buffer full
        }
        _buffer[_head] = item;
        _head = nextHead;
        return true;
    }

    //! Returns an item if one is available.
    [[nodiscard]] std::optional<T> pop() {
        size_t t = _tail.load(std::memory_order_relaxed);
        if (t == _head) {
            return std::nullopt; // empty
        }
        T item = _buffer[t];
        _tail.store((t + 1) % N, std::memory_order_release);
        return item;
    }

    //! Number of items currently stored.
    [[nodiscard]] std::size_t size() const {
        auto tail = _tail.load(std::memory_order_acquire);
        if (_head >= tail) return _head - tail;
        return N - (tail - _head);
    }

    //! Remaining capacity.
    [[nodiscard]] std::size_t availableSpace() const { return N - this->size() - 1; }

private:
    std::array<T, N> _buffer;
    std::atomic<size_t> _tail{0};
    size_t _head = 0; // only modified in push
};

}