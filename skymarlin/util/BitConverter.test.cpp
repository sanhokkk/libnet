#include <catch2/catch_test_macros.hpp>
#include <skymarlin/util/BitConverter.hpp>

namespace skymarlin::util::test {
TEST_CASE("Numeric read and write", "Bitconverter") {
    constexpr size_t buffer_size = 64;
    byte buffer[buffer_size] {};

    size_t wpos {0};
    constexpr uint64_t u_1 {0xa130d'0132'abdb};
    BitConverter::Write(u_1, buffer + wpos);
    wpos += sizeof(uint64_t);

    constexpr float f_1 {12397.12376f};
    BitConverter::Write(f_1, buffer + wpos);
    wpos += sizeof(float);

    constexpr uint8_t b_1 {123};
    BitConverter::Write(b_1, buffer + wpos);
    // wpos += sizeof(byte);

    size_t rpos {0};
    REQUIRE(u_1 == BitConverter::Read<uint64_t>(buffer + rpos));
    rpos += sizeof(uint64_t);

    REQUIRE(f_1 == BitConverter::Read<float>(buffer + rpos));
    rpos += sizeof(float);

    REQUIRE(b_1 == BitConverter::Read<uint8_t>(buffer + rpos));
    // rpos += sizeof(uint8_t);
}
}
