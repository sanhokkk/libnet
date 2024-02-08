#pragma once

#include <cstdint>

namespace skymarlin
{
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

constexpr u8 operator"" _u8(const unsigned long long n)
{
    return static_cast<u8>(n);
}

constexpr u16 operator"" _u16(const unsigned long long n)
{
    return static_cast<u16>(n);
}

constexpr u32 operator"" _u32(const unsigned long long n)
{
    return static_cast<u32>(n);
}

constexpr u64 operator"" _u64(const unsigned long long n)
{
    return static_cast<u64>(n);
}

constexpr i8 operator"" _i8(const unsigned long long n)
{
    return static_cast<i8>(n);
}

constexpr i16 operator"" _i16(const unsigned long long n)
{
    return static_cast<i16>(n);
}

constexpr i32 operator"" _i32(const unsigned long long n)
{
    return static_cast<i32>(n);
}

constexpr i64 operator"" _i64(const unsigned long long n)
{
    return static_cast<i64>(n);
}

constexpr f32 operator"" _f32(const long double f)
{
    return static_cast<f32>(f);
}

constexpr f64 operator"" _f64(const long double f)
{
    return static_cast<f64>(f);
}
}
