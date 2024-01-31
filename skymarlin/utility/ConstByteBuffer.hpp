#pragma once

#include <string>

#include <boost/core/noncopyable.hpp>
#include <boost/asio/buffer.hpp>
#include <skymarlin/utility/BitConverter.hpp>

namespace skymarlin::utility
{
class ConstByteBuffer : boost::noncopyable
{
public:
    ConstByteBuffer(byte* buffer, size_t size);

    ConstByteBuffer(ConstByteBuffer&& x) noexcept;

    template <NumericType T>
    T Read() const;

    template <typename T> requires std::same_as<T, std::string>
    T Read() const;

    void Skip(size_t n) const;

    const byte* data() const { return data_; }

    size_t size() const { return size_; }

    const byte& operator[](size_t pos) const;

    ConstByteBuffer& operator>>(bool& value) const;

    ConstByteBuffer& operator>>(byte& value) const;

    ConstByteBuffer& operator>>(u16& value) const;

    ConstByteBuffer& operator>>(u32& value) const;

    ConstByteBuffer& operator>>(u64& value) const;

    ConstByteBuffer& operator>>(i8& value) const;

    ConstByteBuffer& operator>>(i16& value) const;

    ConstByteBuffer& operator>>(i32& value) const;

    ConstByteBuffer& operator>>(i64& value) const;

    ConstByteBuffer& operator>>(f32& value) const;

    ConstByteBuffer& operator>>(f64& value) const;

    ConstByteBuffer& operator>>(std::string& value) const;

protected:
    byte* data_;
    size_t size_;
    mutable size_t rpos_{0};
};
}
