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
#include <skymarlin/utility/MutableByteStream.hpp>
#include <skymarlin/utility/TypeDefinitions.hpp>

namespace skymarlin::utility
{
TEST(MutableByteStream, Write)
{
    constexpr size_t buffer_size = 256;
    byte src[buffer_size]{};
    auto buffer = MutableByteStream(src, buffer_size);

    constexpr std::string_view sv("I am a string");
    buffer << 42 << sv << 125.12321f;

    i32 i1;
    std::string s;
    f32 f1;
    buffer >> i1 >> s >> f1;

    ASSERT_EQ(42, i1);
    ASSERT_EQ(sv, s);
    ASSERT_EQ(125.12321f, f1);
}

TEST(MutableByteStream, InvalidStringSize)
{
    constexpr size_t buffer_size = 16;
    byte src[buffer_size]{};
    auto buffer = MutableByteStream(src, buffer_size);

    try {
        std::string s {};
        for (size_t i = 0; i < buffer_size + 10; ++i) {
            s += "k";
        }

        buffer << s;

        FAIL(); // Should have thrown error
    } catch (const ByteBufferInvalidValueException& e) {
        std::cout << e.what() << std::endl;
        return;
    }
}
}
