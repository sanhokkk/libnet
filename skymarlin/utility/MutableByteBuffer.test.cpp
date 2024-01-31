#include <gtest/gtest.h>
#include <skymarlin/utility/MutableByteBuffer.hpp>
#include <skymarlin/utility/TypeDefinitions.hpp>

namespace skymarlin::utility
{
TEST(MutableByteBuffer, Write)
{
    constexpr size_t buffer_size = 256;
    byte src[buffer_size]{};
    auto buffer = MutableByteBuffer(src, buffer_size);

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
}
