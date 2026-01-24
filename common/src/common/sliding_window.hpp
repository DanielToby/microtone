#pragma once

#include <deque>

namespace common {

template <typename T>
struct SlidingWindow {
    explicit SlidingWindow(std::size_t maxSize) : _maxSize(maxSize) {}

    [[nodiscard]] bool hasNext() const {
        return _currentIndex < _items.size();
    }

    [[nodiscard]] std::size_t numNext() const {
        return _items.size() - _currentIndex;
    }

    [[nodiscard]] bool hasPrevious() const {
        return _currentIndex > 0;
    }

    void stepForward() {
        if (hasNext()) {
            _currentIndex++;
        }
    }

    void stepBack() {
        if (hasPrevious()) {
            _currentIndex--;
        }
    }

    [[nodiscard]] std::size_t size() const {
        return _items.size();
    }

    [[nodiscard]] std::size_t getCurrentIndex() const {
        return _currentIndex;
    }

    void setCurrentIndex(std::size_t index) {
        _currentIndex = index;
    }

    void push(const T& item) {
        _items.push_back(item);
        if (_items.size() > _maxSize) {
            _items.pop_front();
            _currentIndex--;
        }
    }

    [[nodiscard]] std::vector<T> currentWindow(std::size_t windowSize) const {
        if (_items.empty()) {
            return {};
        }

        const auto bufferSize = _items.size();
        auto front = std::min(_currentIndex, bufferSize - 1);
        auto back  = std::min(front + windowSize, bufferSize);
        return {
            std::next(_items.begin(), front),
            std::next(_items.begin(), back)
        };
    }

private:
    std::deque<T> _items;

    std::size_t _currentIndex = 0;
    std::size_t _maxSize;
};


}