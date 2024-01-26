#pragma once

#include <skymarlin/utility/ConstByteBuffer.hpp>

namespace skymarlin::utility {
class MutableByteBuffer : public ConstByteBuffer {
public:
    explicit MutableByteBuffer(const boost::asio::mutable_buffer& buffer);

    MutableByteBuffer(byte* buffer, size_t size);

    MutableByteBuffer(MutableByteBuffer&& x) noexcept;

    void Clear();

    template<NumericType T>
    void Append(T value);

    template<typename T>
        requires std::same_as<T, std::string_view>
    void Append(T value);

    template<NumericType T>
    void Put(T value, size_t pos);

    byte* data() const;

    byte& operator[](size_t pos) const;

    MutableByteBuffer& operator<<(bool value);

    MutableByteBuffer& operator<<(byte value);

    MutableByteBuffer& operator<<(u16 value);

    MutableByteBuffer& operator<<(u32 value);

    MutableByteBuffer& operator<<(u64 value);

    MutableByteBuffer& operator<<(i8 value);

    MutableByteBuffer& operator<<(i16 value);

    MutableByteBuffer& operator<<(i32 value);

    MutableByteBuffer& operator<<(i64 value);

    MutableByteBuffer& operator<<(f32 value);

    MutableByteBuffer& operator<<(f64 value);

    MutableByteBuffer& operator<<(std::string_view value);

    MutableByteBuffer& operator<<(const std::string& value);

private:
    mutable size_t wpos_{0};
};
}
