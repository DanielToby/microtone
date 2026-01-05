#pragma once

#include <array>
#include <atomic>
#include <functional>

namespace common {

namespace audio {
//! Describes each block in the ring buffer in case producing T's is more efficient if done in blocks.
//! At a sample rate of 44.1 kHz, this is 512 / 48000, or ~10.7 ms  of audio.
constexpr std::size_t AudioBlockSize = 512;
using SampleT = float;
using FrameBlock = std::array<SampleT, AudioBlockSize>;
constexpr auto emptyFrameBlock = FrameBlock{0.f};

[[nodiscard]] inline double getDuration_us(const std::size_t blockSize, const double sampleRate) {
    return (static_cast<double>(blockSize) / sampleRate) * 1e6;
}

}

//! Provides thread-safe access to a buffer of Ts.
//! Buffering helps avoid jitter in multithreaded contexts. As long as the producer is more often faster than the consumer,
//! the buffer helps accommodate temporary slowdowns, which prevents jitter. This comes at the cost of latency, because
//! the consumer is popping items that are N * blockTime_s behind the producer.
//! A default of 4 is used with FrameBlock for a total latency of ~42.8 ms.
template <typename T, size_t N = 4>
class RingBuffer {
    static_assert(N >= 2);
    static_assert(std::is_trivially_copyable_v<T>);
public:
    RingBuffer() = default;

    [[nodiscard]] bool isFull() const noexcept {
        return nextAfter(_head.load(std::memory_order_relaxed)) == _tail.load(std::memory_order_relaxed);
    }

    //! Returns true if an item was pushed.
    [[nodiscard]] bool push(const T& item) noexcept {
        const auto head = _head.load(std::memory_order_relaxed);
        const auto tail = _tail.load(std::memory_order_relaxed);
        const auto nextHead = nextAfter(head);
        if (nextHead != tail) {
            _buffer[head] = item;
            _head.store(nextHead, std::memory_order_release);
            return true;
        }
        return false;
    }

    //! If an item is available, `fn` is invoked on it.
    [[nodiscard]] bool pop(const std::function<void(const T&)>& fn) noexcept {
        const auto head = _head.load(std::memory_order_acquire);
        const auto tail = _tail.load(std::memory_order_relaxed);
        if (head != tail) {
            std::invoke(fn, _buffer[tail]);
            const auto nextTail = nextAfter(tail);
            _tail.store(nextTail, std::memory_order_release);
            return true;
        }
        return false;
    }

private:
    [[nodiscard]] static constexpr std::size_t nextAfter(std::size_t i) noexcept {
        return (i + 1) % N;
    }

    std::array<T, N> _buffer;
    alignas(64) std::atomic<std::size_t> _tail{0};
    alignas(64) std::atomic<std::size_t> _head{0};
};

}