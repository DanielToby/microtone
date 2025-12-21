#pragma once

#include <stdexcept>
#include <string>

namespace common {
struct MicrotoneException : public std::runtime_error {
    explicit MicrotoneException(const std::string& msg);
};
}
