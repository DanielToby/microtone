#pragma once

#include <atomic>
#include <array>
#include <cstddef>
#include <optional>

namespace io {

//! Provides thread-safe access to a buffer of Ts.
//! This class could be improved using batch operations to reduce atomic operations.
template <typename T, size_t N>
class RingBuffer {
public:
    bool push(const T& item) {
        size_t nextHead = (_head + 1) % N;
        if (nextHead == _tail.load(std::memory_order_acquire)) {
            return false; // buffer full
        }
        _buffer[_head] = item;
        _head = nextHead;
        return true;
    }

    std::optional<T> pop() {
        size_t t = _tail.load(std::memory_order_relaxed);
        if (t == _head) {
            return std::nullopt; // empty
        }
        T item = _buffer[t];
        _tail.store((t + 1) % N, std::memory_order_release);
        return item;
    }

private:
    std::array<T, N> _buffer;
    std::atomic<size_t> _tail{0};
    size_t _head = 0; // only modified in push
};

}