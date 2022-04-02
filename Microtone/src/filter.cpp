#include <microtone/filter.hpp>

namespace microtone {


class Filter::impl {
public:
    impl() :
        _lastSample{0},
        _alpha{0.99} {}

    impl(const impl& other) :
        _lastSample{other._lastSample},
        _alpha{other._alpha} {}

    float nextSample(float in) {
        auto out = _alpha * _lastSample + (1.0 - _alpha) * in;
        _lastSample = out;
        return out;
    }

    float _lastSample;
    double _alpha;
};

Filter::Filter() :
    _impl{new impl{}} {
}

Filter::Filter(const Filter& other) :
    _impl{new impl{*other._impl}} {
}

Filter::Filter(Filter&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Filter& Filter::operator=(Filter&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

float Filter::nextSample(float in) {
    return _impl->nextSample(in);
}

Filter::~Filter() = default;

}
