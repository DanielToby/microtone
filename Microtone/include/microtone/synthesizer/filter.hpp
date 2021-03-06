#pragma once

#include <microtone/microtone_platform.hpp>

#include <memory>

namespace microtone {

class Filter {
public:
    Filter();
    Filter(const Filter&);
    Filter(Filter&&) noexcept;
    Filter& operator=(const Filter&) noexcept;
    Filter& operator=(Filter&&) noexcept;
    ~Filter();

    float nextSample(float in);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
