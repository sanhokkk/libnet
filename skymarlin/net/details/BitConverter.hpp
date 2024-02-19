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
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
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
#include <cstdint>

namespace skymarlin
{
using byte = uint8_t;
}

namespace skymarlin::net::details
{
template <typename T>
concept NumericType = std::is_arithmetic_v<T>;

class BitConverter
{
public:
    template <NumericType T>
    static T Convert(byte* src)
    {
        AlignEndianness(src, sizeof(T));

        T value {};
        std::memcpy(&value, src, sizeof(T));
        return value;
    }

    template <NumericType T>
    static void Convert(const T value, byte* dest)
    {
        std::memcpy(dest, &value, sizeof(T));
        AlignEndianness(dest, sizeof(T));
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
