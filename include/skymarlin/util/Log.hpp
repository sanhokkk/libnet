#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>

#include <iostream>

namespace skymarlin::util
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


class Log final
{
public:
    static void Redirect(std::ostream& os)
    {
        const auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(os);
        spdlog::default_logger()->sinks().clear();
        spdlog::default_logger()->sinks().push_back(ostream_sink);
    }
};
}