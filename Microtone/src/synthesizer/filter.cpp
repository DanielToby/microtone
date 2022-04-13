#include <microtone/synthesizer/filter.hpp>

namespace microtone {

class Filter::impl {
public:
    impl() :
        _lastSample{0},
        _alpha{0.5} {}

    impl(const impl& other) :
        _lastSample{other._lastSample},
        _alpha{other._alpha} {}

    float nextSample(float in) {
        auto out = static_cast<float>(_alpha * _lastSample + (1.0 - _alpha) * in);
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

Filter& Filter::operator=(const Filter& other) noexcept {
    _impl = std::make_unique<impl>(*other._impl);
    return *this;
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
