#include <gtest/gtest.h>
#include <skymarlin/utility/MutableByteBuffer.hpp>
#include <skymarlin/utility/TypeDefinitions.hpp>

namespace skymarlin::utility {
TEST(MutableByteBuffer, Write) {
    byte src[256] {};
    auto buffer = MutableByteBuffer(src, 4);

    std::string_view sv("I am a string");
    buffer << (i32)42 << sv << (f32)125.12321f;

    i32 i1;
    std::string s;
    f32 f1;
    buffer >> i1 >> s >> f1;
    
    ASSERT_EQ((i32)42, i1);
    ASSERT_EQ("I am a string", s);
    ASSERT_EQ((f32)125.12321f, f1);
}  
}

