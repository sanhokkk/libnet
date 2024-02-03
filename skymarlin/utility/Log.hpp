#pragma once

#include <spdlog/spdlog.h>

namespace skymarlin::utility
{
#define SKYMARLIN_LOG_INFO(format, ...) \
    spdlog::info(format __VA_OPT__(,) __VA_ARGS__)

#define SKYMARLIN_LOG_ERROR(format, ...) \
    spdlog::error(format __VA_OPT__(,) __VA_ARGS__)

#define SKYMARLIN_LOG_WARN(format, ...) \
    spdlog::warn(format __VA_OPT__(,) __VA_ARGS__)

#define SKYMARLIN_LOG_CRITICAL(format, ...) \
    spdlog::critical(format __VA_OPT__(,) __VA_ARGS__)

#define SKYMARLIN_LOG_DEBUG(format, ...) \
    spdlog::debug(format __VA_OPT__(,) __VA_ARGS__)

//TODO: `format_string` cannot be passed; it should be a constant expression.
/*class Log final
{
public:
    template <typename... Args>
    static void Info(std::string_view format_string, Args... args)
    {
        spdlog::info(format_string, args...);
    }

    template <typename... Args>
    static void Error(constexpr std::string_view format_string, Args... args)
    {
        spdlog::error(format_string, args...);
    }

    template <typename... Args>
    static void Warn(constexpr std::string_view format_string, Args... args)
    {
        spdlog::warn(format_string, args...);
    }

    template <typename... Args>
    static void Critical(constexpr std::string_view format_string, Args... args)
    {
        spdlog::critical(format_string, args...);
    }

    template <typename... Args>
    static void Debug(constexpr std::string_view format_string, Args... args)
    {
        spdlog::debug(format_string, args...);
    }
};*/
}
