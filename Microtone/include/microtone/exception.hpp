#pragma once

#include <microtone/microtone_platform.hpp>

#include <stdexcept>

namespace microtone {
struct MicrotoneException : public std::runtime_error {
    explicit MicrotoneException(const std::string& msg);
};
}
