#include <common/exception.hpp>

#include <common/log.hpp>

namespace common {

MicrotoneException::MicrotoneException(const std::string& msg) :
    std::runtime_error(msg) {
    M_ERROR("MicrotoneException: {0}", msg);
};

}
