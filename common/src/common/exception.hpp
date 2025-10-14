#pragma once

#include <stdexcept>

namespace common {
struct MicrotoneException : public std::runtime_error {
    explicit MicrotoneException(const std::string& msg);
};
}
