#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace skymarlin {
using byte = uint8_t;
}

namespace skymarlin::util {
template <typename T>
concept NumericType = std::is_arithmetic_v<T>;

class BitConverter {
public:
    template <NumericType T>
    static T Read(const byte* src) {
        T value {};
        std::memcpy(&value, src, sizeof(T));
        AlignEndianness(reinterpret_cast<byte*>(&value), sizeof(T));

        return value;
    }

    template <NumericType T>
    static void Write(const T value, byte* dest) {
        std::memcpy(dest, &value, sizeof(T));
        AlignEndianness(dest, sizeof(T));
    }

    template <typename T>
    requires std::is_same_v<T, std::string_view>
    static size_t BytesSize(T src) {
        //TODO: encoding consideration
        return src.size();
    }

private:
    static void AlignEndianness(byte* src, const size_t n) {
        if constexpr (std::endian::native == std::endian::little) {
            return;
        }

        std::reverse(src, src + n);
    }
};
}
