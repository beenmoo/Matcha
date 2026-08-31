#pragma once

#include <format>
#include <string>

#define MT_CORE_LOGGER "MatchaCoreLogger"
#define MT_CLIENT_LOGGER "MatchaClientLogger"

namespace Matcha
{
class Logger
{
public:
    Logger();
    ~Logger();
};

namespace Log
{
void CoreTrace(const std::string& message);
void CoreInfo(const std::string& message);
void CoreWarn(const std::string& message);
void CoreError(const std::string& message);
void CoreCritical(const std::string& message);

void Trace(const std::string& message);
void Info(const std::string& message);
void Warn(const std::string& message);
void Error(const std::string& message);
void Critical(const std::string& message);
}  // namespace Log
}  // namespace Matcha

#ifdef MT_DEBUG
#define MT_CORE_TRACE(...) ::Matcha::Log::CoreTrace(std::format(__VA_ARGS__))
#define MT_CORE_INFO(...) ::Matcha::Log::CoreInfo(std::format(__VA_ARGS__))
#define MT_CORE_WARN(...) ::Matcha::Log::CoreWarn(std::format(__VA_ARGS__))
#define MT_CORE_ERROR(...) ::Matcha::Log::CoreError(std::format(__VA_ARGS__))
#define MT_CORE_CRITICAL(...) ::Matcha::Log::CoreCritical(std::format(__VA_ARGS__))

#define MT_TRACE(...) ::Matcha::Log::Trace(std::format(__VA_ARGS__))
#define MT_INFO(...) ::Matcha::Log::Info(std::format(__VA_ARGS__))
#define MT_WARN(...) ::Matcha::Log::Warn(std::format(__VA_ARGS__))
#define MT_ERROR(...) ::Matcha::Log::Error(std::format(__VA_ARGS__))
#define MT_CRITICAL(...) ::Matcha::Log::Critical(std::format(__VA_ARGS__))
#else
#define MT_CORE_TRACE(...)
#define MT_CORE_INFO(...)
#define MT_CORE_WARN(...)
#define MT_CORE_ERROR(...)
#define MT_CORE_CRITICAL(...)

#define MT_TRACE(...)
#define MT_INFO(...)
#define MT_WARN(...)
#define MT_ERROR(...)
#define MT_CRITICAL(...)
#endif
