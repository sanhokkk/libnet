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
