#include "Logger.h"

#include <vector>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Matcha
{
Logger::Logger()
{
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Logs/Matcha.log", true));
    logSinks[0]->set_pattern("%^[%T] %n: %v%$");
    logSinks[1]->set_pattern("[%T] [%l] %n: %v");

    auto coreLogger = std::make_shared<spdlog::logger>(MT_CORE_LOGGER, begin(logSinks), end(logSinks));
    spdlog::register_logger(coreLogger);
    coreLogger->set_level(spdlog::level::trace);
    coreLogger->flush_on(spdlog::level::trace);

    auto clientLogger = std::make_shared<spdlog::logger>(MT_CLIENT_LOGGER, begin(logSinks), end(logSinks));
    spdlog::register_logger(clientLogger);
    clientLogger->set_level(spdlog::level::trace);
    clientLogger->flush_on(spdlog::level::trace);
}

Logger::~Logger()
{
    spdlog::shutdown();
}
}  // namespace Matcha