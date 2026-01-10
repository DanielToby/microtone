#include <common/log.hpp>

#include "spdlog/sinks/rotating_file_sink.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>

namespace common {

namespace {

[[nodiscard]] std::string defaultMacOSLogPath() {
    const char* home = std::getenv("HOME");
    return fmt::format("{}/Library/Logs/microtone/microtone.log", home ? home : ".");
}

[[nodiscard]] std::string defaultWindowsLogPath() {
    if (const char* localAppData = std::getenv("LOCALAPPDATA")) {
        return fmt::format(R"({}\microtone\logs\microtone.log)", localAppData);
    }
    if (const char* appData = std::getenv("APPDATA")) {
        return fmt::format(R"({}\microtone\logs\microtone.log)", appData);
    }
    return "microtone.log";
}

[[nodiscard]] std::string defaultLinuxLogPath() {
    const char* state = std::getenv("XDG_STATE_HOME");
    const char* home  = std::getenv("HOME");
    auto stateFallback = fmt::format("{}/.local/state", home ? home : ".");
    return fmt::format("{}/microtone/microtone.log", state ? state : stateFallback);
}

[[nodiscard]] std::string defaultLogPath() {
#if defined(__APPLE__)
    return defaultMacOSLogPath();
#elif defined(_WIN32)
    return defaultWindowsLogPath();
#elif defined(__linux__)
    return defaultLinuxLogPath();
#else
    static_assert(false); //< Unknown platform.
#endif
}

}

std::shared_ptr<spdlog::logger> Log::s_Logger = nullptr;

void Log::init(bool enableConsoleLogging) {
    auto sinks = std::vector<spdlog::sink_ptr>{};

    if (enableConsoleLogging) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[microtone] [%^%l%$] %v");
        sinks.emplace_back(console_sink);
    }

    const auto log_path = defaultLogPath();
    std::filesystem::create_directories(std::filesystem::path(log_path).parent_path());

    constexpr size_t max_file_size = 5 * 1024 * 1024; // 5 MB
    constexpr size_t max_files = 3;

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        log_path, max_file_size, max_files);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("[microtone] [%^%l%$] %v");
    sinks.emplace_back(file_sink);

    s_Logger = std::make_shared<spdlog::logger>("microtone_logger", sinks.begin(), sinks.end());
}

std::shared_ptr<spdlog::logger> Log::getLogger() {
    return s_Logger;
}

std::string Log::getDefaultLogfilePath() {
    return defaultLogPath();
}

void Log::shutdown() {
    spdlog::shutdown();
    s_Logger.reset();
}

};
