#pragma once

#include <atomic>
#include <string>

#include "common/exception.hpp"

namespace common {

//! NumReaders must be known at compile time to prevent this class from allocating.
template <std::size_t NumReaders>
struct DirtyFlag {
    DirtyFlag() = default;

    void setDirty() noexcept {
        for (auto& _flag : _flags) {
            _flag.store(true, std::memory_order_release);
        }
    }

    [[nodiscard]] bool isDirty(std::size_t readerId) const noexcept {
        return _flags[readerId].load(std::memory_order_acquire);
    }

    void markRead(std::size_t readerId) noexcept {
        _flags[readerId].store(false, std::memory_order_release);
    }

    //! Throws if NumReaders readers are already registered.
    [[nodiscard]] std::size_t registerReader() noexcept(false) {
        const auto readerId = _numRegisteredReaders.fetch_add(1, std::memory_order_release);
        if (readerId >= NumReaders) {
            throw MicrotoneException("Exceeded the number of allowable readers.");
        }
        return readerId;
    }

private:
    std::atomic<std::size_t> _numRegisteredReaders{0};
    std::array<std::atomic<bool>, NumReaders> _flags{true}; //< No reads yet, this is considered dirty.
};

//! Tracks access to T. Requires that "read" and "write" and typename ValueType exist on T.
template <typename T, std::size_t NumReaders>
class DirtyFlagged {
public:
    using ValueType = typename T::ValueType;

    DirtyFlagged() = default;

    //! Registers a new reader. Callers should use the returned ID to access the other methods.
    //! Warning: this function allocates! Don't call it in realtime contexts!
    [[nodiscard]] std::size_t registerReader() const noexcept(false) {
        return _dirtyFlag.registerReader();
    }

    //! Reads the current value without affecting dirty-ness.
    [[nodiscard]] ValueType quietRead() const {
        return _value.read();
    }

    [[nodiscard]] ValueType read(std::size_t readerId) const {
        _dirtyFlag.markRead(readerId);
        return _value.read();
    }

    void write(ValueType&& value) {
        _value.write(std::forward<ValueType>(value));
        _dirtyFlag.setDirty();
    }

    [[nodiscard]] bool isDirty(std::size_t readerId) const {
        return _dirtyFlag.isDirty(readerId);
    }

private:
    T _value;
    mutable DirtyFlag<NumReaders> _dirtyFlag;
};

}
