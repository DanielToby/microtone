#include <microtone/exception.hpp>
#include <microtone/log.hpp>

namespace microtone {

MicrotoneException::MicrotoneException(const std::string& msg) :
    std::runtime_error(msg) {
    M_ERROR("MicrotoneException: {0}", msg);
};

}
