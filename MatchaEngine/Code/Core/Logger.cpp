#include "Logger.h"

#include <vector>
#include <memory>
#include <spdlog/spdlog.h>
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

namespace
{
void LogTo(const char* loggerName, spdlog::level::level_enum level, const std::string& message)
{
    if (auto logger = spdlog::get(loggerName))
        logger->log(level, message);
}
}  // namespace

namespace Log
{
void CoreTrace(const std::string& message)
{
    LogTo(MT_CORE_LOGGER, spdlog::level::trace, message);
}

void CoreInfo(const std::string& message)
{
    LogTo(MT_CORE_LOGGER, spdlog::level::info, message);
}

void CoreWarn(const std::string& message)
{
    LogTo(MT_CORE_LOGGER, spdlog::level::warn, message);
}

void CoreError(const std::string& message)
{
    LogTo(MT_CORE_LOGGER, spdlog::level::err, message);
}

void CoreCritical(const std::string& message)
{
    LogTo(MT_CORE_LOGGER, spdlog::level::critical, message);
}

void Trace(const std::string& message)
{
    LogTo(MT_CLIENT_LOGGER, spdlog::level::trace, message);
}

void Info(const std::string& message)
{
    LogTo(MT_CLIENT_LOGGER, spdlog::level::info, message);
}

void Warn(const std::string& message)
{
    LogTo(MT_CLIENT_LOGGER, spdlog::level::warn, message);
}

void Error(const std::string& message)
{
    LogTo(MT_CLIENT_LOGGER, spdlog::level::err, message);
}

void Critical(const std::string& message)
{
    LogTo(MT_CLIENT_LOGGER, spdlog::level::critical, message);
}
}  // namespace Log
}  // namespace Matcha
