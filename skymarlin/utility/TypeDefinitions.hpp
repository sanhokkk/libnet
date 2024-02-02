#pragma once

#include <cstdint>

namespace skymarlin {
using byte = uint8_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

using StringLengthType = u16;
constexpr static size_t STRING_HEADER_SIZE = sizeof(StringLengthType);
}
