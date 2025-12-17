#pragma once

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace common {

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

    static std::string getDefaultLogfilePath();

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

}

#define M_TRACE(...) ::common::Log::getLogger()->trace(__VA_ARGS__)
#define M_DEBUG(...) ::common::Log::getLogger()->debug(__VA_ARGS__)
#define M_INFO(...) ::common::Log::getLogger()->info(__VA_ARGS__)
#define M_WARN(...) ::common::Log::getLogger()->warn(__VA_ARGS__)
#define M_ERROR(...) ::common::Log::getLogger()->error(__VA_ARGS__)
#define M_CRITICAL(...) ::common::Log::getLogger()->critical(__VA_ARGS__)
