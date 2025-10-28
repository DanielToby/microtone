#pragma once

#include <atomic>
#include <array>
#include <cstddef>
#include <optional>

namespace common::audio {

//! Describes each block in the ring buffer in case producing T's is more efficient if done in blocks.
//! At a sample rate of 44.1 kHz, this is 512 / 48000, or ~10.7 ms  of audio.
constexpr std::size_t AudioBlockSize = 512;
using SampleT = float;
using FrameBlock = std::array<SampleT, AudioBlockSize>;

//! Provides thread-safe access to a buffer of Ts.
//! Buffering helps avoid jitter in multithreaded contexts. As long as the producer is more often faster than the consumer,
//! the buffer helps accommodate temporary slowdowns, which prevents jitter. This comes at the cost of latency, because
//! the consumer is popping items that are N * blockTime_s behind the producer.
//! A default of 4 is used with FrameBlock for a total latency of ~42.8 ms.
template <typename T = FrameBlock, size_t N = 4>
class RingBuffer {
public:
    RingBuffer() = default;

    [[nodiscard]] bool isFull() const {
        return this->nextHead() == _tail.load(std::memory_order_acquire);
    }

    //! Returns true if an item was pushed.
    [[nodiscard]] bool push(const T& item) {
        if (auto nextHead = this->nextHead(); nextHead != _tail.load(std::memory_order_acquire)) {
            _buffer[nextHead] = item;
            _head.store(nextHead);
            return true;
        }
        return false;
    }

    //! Returns an item if one is available.
    [[nodiscard]] std::optional<T> pop() {
        size_t tail = _tail.load(std::memory_order_relaxed);
        if (tail == _head.load(std::memory_order_relaxed)) {
            return std::nullopt; // empty
        }
        T item = _buffer[tail];
        _tail.store((tail + 1) % N, std::memory_order_release);
        _lastPoppedIndex.store(tail, std::memory_order_release);
        return item;
    }

    //! Returns the last item returned by pop().
    [[nodiscard]] std::optional<T> lastPopped() const {
        if (auto lastPoppedIndex = _lastPoppedIndex.load(std::memory_order_relaxed)) {
            return _buffer[*lastPoppedIndex];
        }
        return std::nullopt;
    }

private:
    [[nodiscard]] std::size_t nextHead() const {
        return (_head.load(std::memory_order_acquire) + 1) % N;
    }

    std::array<T, N> _buffer;
    std::atomic<size_t> _tail{0};
    std::atomic<size_t> _head{0};
    std::atomic<std::optional<size_t>> _lastPoppedIndex{std::nullopt};
};

}