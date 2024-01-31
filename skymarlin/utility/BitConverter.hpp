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

    /**
     * \brief Convert byte buffer to a value of type T
     */
    template <NumericType T>
    [[nodiscard]] static T Convert(byte* buffer)
    {
        AlignEndianess(buffer, sizeof(T));

        T value{};
        std::memcpy(&value, buffer, sizeof(T));
        return value;
    }

    /**
     * \brief Convert a value of type T into byte buffer
     */
    template <NumericType T>
    static void Convert(const T value, byte* buffer)
    {
        /*constexpr T mask = static_cast<T>(0b1111'1111);
        for (size_t i{0}; i < sizeof(T); ++i) {
            buffer[i] = static_cast<byte>((value >> i * 8) & mask);
        }*/
        std::memcpy(buffer, &value, sizeof(T));
        AlignEndianess(buffer, sizeof(T));
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
