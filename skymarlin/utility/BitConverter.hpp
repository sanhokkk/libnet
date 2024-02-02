#pragma once

#include <algorithm>
#include <bit>
#include <cstring>

#include <skymarlin/utility/TypeDefinitions.hpp>

namespace skymarlin::utility
{
template <typename T>
concept NumericType = std::is_arithmetic_v<T>;

class BitConverter
{
public:
    enum class Encoding
    {
        UTF_8,
        UTF_16,
    };

    template <NumericType T>
    [[nodiscard]] static T Convert(byte* src)
    {
        AlignEndianess(src, sizeof(T));

        T value{};
        std::memcpy(&value, src, sizeof(T));
        return value;
    }

    template <typename T> requires std::is_same_v<T, std::string>
    [[nodiscard]] static T Convert(byte* src)
    {
        AlignEndianess(src, sizeof(T));

        T value{};
        std::memcpy(&value, src, sizeof(T));
        return value;
    }

    template <NumericType T>
    static void Convert(const T value, byte* dest)
    {
        std::memcpy(dest, &value, sizeof(T));
        AlignEndianess(dest, sizeof(T));
    }



    //TODO: Character encoding
    static size_t GetStringBytesSize(std::string_view s)
    {
        return s.size() + sizeof(STRING_HEADER_SIZE);
    }

private:
    static void AlignEndianess(byte* src, const size_t n)
    {
        if constexpr (std::endian::native == std::endian::little) {
            return;
        }

        std::reverse(src, src + n);
    }
};
}
