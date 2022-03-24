#pragma once

#include <stdexcept>

namespace microtone {
struct MicrotoneException : public std::runtime_error {
    explicit MicrotoneException(const std::string& msg);
};
}
