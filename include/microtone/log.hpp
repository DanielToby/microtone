#pragma once

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace microtone {

class Log {
public:
    enum class LogLevel {
        trace,
        debug,
        info,
        warn,
        error,
        critical
    };

    static void init();
    static std::shared_ptr<spdlog::logger> getLogger();

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

}

#define M_TRACE(...) ::microtone::Log::getLogger()->trace(__VA_ARGS__)
#define M_DEBUG(...) ::microtone::Log::getLogger()->debug(__VA_ARGS__)
#define M_INFO(...) ::microtone::Log::getLogger()->info(__VA_ARGS__)
#define M_WARN(...) ::microtone::Log::getLogger()->warn(__VA_ARGS__)
#define M_ERROR(...) ::microtone::Log::getLogger()->error(__VA_ARGS__)
#define M_CRITICAL(...) ::microtone::Log::getLogger()->critical(__VA_ARGS__)

#ifdef _WIN32
#define API_CALL() E_TRACE("Called " __FUNCTION__ "() : " __FILE__ ":{}", __LINE__)
#else
#define API_CALL()
#endif
