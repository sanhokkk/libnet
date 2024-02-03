#pragma once

#include <format>
#include <iostream>
#include <string_view>

namespace skymarlin::utility
{
using std::literals::operator ""sv;

class Log final
{
public:
    template <typename... Args>
    static void Info(std::string_view format, Args... args)
    {
        std::cout << std::format(format, args...) << std::endl;
    }

    template <typename... Args>
    static void Error(std::string_view format, Args... args)
    {
        std::cout << std::format(format, args...) << std::endl;
    }

    template <typename... Args>
    static void Warn(std::string_view format, Args... args)
    {
        std::cout << std::format(format, args...) << std::endl;
    }

    template <typename... Args>
    static void Critical(std::string_view format, Args... args)
    {
        std::cout << std::format(format, args...) << std::endl;
    }

    template <typename... Args>
    static void Debug(std::string_view format, Args... args)
    {
        std::cout << std::format(format, args...) << std::endl;
    }
};
}
