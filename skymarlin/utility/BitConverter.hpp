/*
 * MIT License
 *
 * Copyright (c) 2024 skymarlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <bit>
#include <cstring>

#include <skymarlin/utility/TypeDefinitions.hpp>

namespace skymarlin::utility
{
using StringLengthType = u16;
constexpr static size_t STRING_HEADER_SIZE = sizeof(StringLengthType);

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
    static T Convert(byte* src)
    {
        AlignEndianness(src, sizeof(T));

        T value {};
        std::memcpy(&value, src, sizeof(T));
        return value;
    }

    template <typename T> requires std::same_as<T, std::string>
    static T Convert(byte* src)
    {
        std::string value {};
        size_t pos {0};

        const auto length = Convert<StringLengthType>(src);
        pos += sizeof(StringLengthType);

        while (pos < sizeof(StringLengthType) + length) {
            //TODO: Character encoding
            value += Convert<char>(src);
            pos += sizeof(char);
        }

        return value;
    }

    template <NumericType T>
    static void Convert(const T value, byte* dest)
    {
        std::memcpy(dest, &value, sizeof(T));
        AlignEndianness(dest, sizeof(T));
    }

    template <typename T> requires std::same_as<T, std::string_view>
    static void Convert(const T value, byte* dest)
    {
        size_t pos {0};

        const auto length = value.length();
        Convert<StringLengthType>(length, dest + pos);
        pos += sizeof(StringLengthType);

        //TODO: Character encoding
        for (const char c : value) {
            Convert(c, dest + pos);
            pos += sizeof(char);
        }
    }

private:
    static void AlignEndianness(byte* src, const size_t n)
    {
        if constexpr (std::endian::native == std::endian::little) {
            return;
        }

        std::reverse(src, src + n);
    }
};
}
