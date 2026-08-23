#pragma once

#include <spdlog/spdlog.h>

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
}  // namespace Matcha

#ifdef MT_DEBUG
#define MT_CORE_TRACE(...)                               \
    if (spdlog::get(MT_CORE_LOGGER))                     \
    {                                                    \
        spdlog::get(MT_CORE_LOGGER)->trace(__VA_ARGS__); \
    }
#define MT_CORE_INFO(...)                               \
    if (spdlog::get(MT_CORE_LOGGER))                    \
    {                                                   \
        spdlog::get(MT_CORE_LOGGER)->info(__VA_ARGS__); \
    }
#define MT_CORE_WARN(...)                               \
    if (spdlog::get(MT_CORE_LOGGER))                    \
    {                                                   \
        spdlog::get(MT_CORE_LOGGER)->warn(__VA_ARGS__); \
    }
#define MT_CORE_ERROR(...)                               \
    if (spdlog::get(MT_CORE_LOGGER))                     \
    {                                                    \
        spdlog::get(MT_CORE_LOGGER)->error(__VA_ARGS__); \
    }
#define MT_CORE_CRITICAL(...)                               \
    if (spdlog::get(MT_CORE_LOGGER))                        \
    {                                                       \
        spdlog::get(MT_CORE_LOGGER)->critical(__VA_ARGS__); \
    }

#define MT_TRACE(...)                                      \
    if (spdlog::get(MT_CLIENT_LOGGER))                     \
    {                                                      \
        spdlog::get(MT_CLIENT_LOGGER)->trace(__VA_ARGS__); \
    }
#define MT_INFO(...)                                       \
    if (spdlog::get(MT_CLIENT_LOGGER))                     \
    {                                                      \
        spdlog::get(MT_CLIENT_LOGGER)->trace(__VA_ARGS__); \
    }
#define MT_WARN(...)                                       \
    if (spdlog::get(MT_CLIENT_LOGGER))                     \
    {                                                      \
        spdlog::get(MT_CLIENT_LOGGER)->trace(__VA_ARGS__); \
    }
#define MT_ERROR(...)                                      \
    if (spdlog::get(MT_CLIENT_LOGGER))                     \
    {                                                      \
        spdlog::get(MT_CLIENT_LOGGER)->trace(__VA_ARGS__); \
    }
#define MT_CRITICAL(...)                                   \
    if (spdlog::get(MT_CLIENT_LOGGER))                     \
    {                                                      \
        spdlog::get(MT_CLIENT_LOGGER)->trace(__VA_ARGS__); \
    }
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