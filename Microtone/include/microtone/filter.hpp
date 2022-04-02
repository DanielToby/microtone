#pragma once

#include <microtone/microtone_platform.hpp>

#include <memory>

namespace microtone {

class Filter {
public:
    explicit Filter();
    Filter(const Filter&);
    Filter& operator=(const Filter&) = delete;
    Filter(Filter&&) noexcept;
    Filter& operator=(Filter&&) noexcept;
    ~Filter();

    float nextSample(float in);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
