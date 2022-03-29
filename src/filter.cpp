#include <microtone/filter.hpp>

namespace microtone {


class Filter::impl {
public:
    impl() {}

    impl(const impl& other) {
    }
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

Filter::~Filter() = default;

}
