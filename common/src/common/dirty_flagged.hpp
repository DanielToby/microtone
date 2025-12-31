#pragma once

#include <atomic>

namespace common {

//! Tracks access to T. Requires that "read" and "write" and typename ValueType exist on T.
template <typename T>
class DirtyFlagged {
public:
    using ValueType = typename T::ValueType;

    DirtyFlagged() = default;
    explicit DirtyFlagged(const T& value) :
        _value(value) {}

    [[nodiscard]] ValueType read() const {
        auto result = _value.read();
        _isDirty.store(false, std::memory_order_release);
        return result;
    }

    void write(ValueType&& value) {
        _value.write(std::forward<ValueType>(value));
        _isDirty.store(true, std::memory_order_release);
    }

    [[nodiscard]] bool isDirty() const {
        return _isDirty.load(std::memory_order_acquire);
    }

private:
    T _value;
    mutable std::atomic<bool> _isDirty{false};
};

}
