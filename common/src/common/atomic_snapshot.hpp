#pragma once

#include <atomic>

namespace common {

//! Provides thread safe access to an instance of T.
template <typename T>
class AtomicSnapshot {
public:
    void write(const T& new_value) noexcept {
        // Producer writes to back buffer
        back = new_value;
        // Atomically publish pointer to the new version
        front.store(&back, std::memory_order_release);
    }

    T read() const noexcept {
        // Consumer reads whatever front points to
        const T* ptr = front.load(std::memory_order_acquire);
        return *ptr;
    }

private:
    T back;
    std::atomic<const T*> front{&back};
};

}