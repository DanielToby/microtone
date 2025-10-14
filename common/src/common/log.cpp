#include <common/log.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace common {

std::shared_ptr<spdlog::logger> Log::s_Logger = nullptr;

void Log::init() {
    auto sinks = std::vector<spdlog::sink_ptr>{};
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("[microtone] [%^%l%$] %v");
    sinks.emplace_back(console_sink);

    s_Logger = std::make_shared<spdlog::logger>("microtone_logger", sinks.begin(), sinks.end());
}

std::shared_ptr<spdlog::logger> Log::getLogger() {
    return s_Logger;
}

};
