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

#include <gtest/gtest.h>
#include <skymarlin/utility/BitConverter.hpp>

namespace skymarlin::utility
{
TEST(BitConverter, NumericReadWrite)
{
    constexpr size_t buffer_size = 64;
    byte buffer[buffer_size]{};

    size_t wpos{0};
    constexpr u64 u_1{0xa130d'0132'abdb};
    BitConverter::Convert<u64>(u_1, buffer + wpos);
    wpos += sizeof(u64);

    constexpr f32 f_1{12397.12376f};
    BitConverter::Convert<f32>(f_1, buffer + wpos);
    wpos += sizeof(f32);

    constexpr byte b_1{123};
    BitConverter::Convert<byte>(b_1, buffer + wpos);
    // wpos += sizeof(byte);

    size_t rpos{0};
    ASSERT_EQ(u_1, BitConverter::Convert<u64>(buffer + rpos));
    rpos += sizeof(u64);

    ASSERT_EQ(f_1, BitConverter::Convert<f32>(buffer + rpos));
    rpos += sizeof(f32);

    ASSERT_EQ(b_1, BitConverter::Convert<byte>(buffer + rpos));
    // rpos += sizeof(byte);
}
}
