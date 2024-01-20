#pragma once

#include <string>
#include <skymarlin/network/TypeDefinitions.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/asio/buffer.hpp>

namespace skymarlin::network::packet {
template<typename T>
concept NumericType = std::is_arithmetic_v<T>;

class ConstByteBuffer : boost::noncopyable {
public:
    // explicit ConstByteBuffer(const boost::asio::const_buffer& buffer);

    ConstByteBuffer(byte* buffer, size_t size);

    template<NumericType T>
    T Read() const;

    // TODO: Provide Read<std::string_view> <- How can I view byte array?
    template<typename T>
        requires std::same_as<T, std::string>
    T Read() const;

    template<NumericType T>
    T Read(size_t pos) const;

    void Read(byte* dest, size_t n) const;

    void Skip(size_t skip_size) const;

    const byte* data() const;

    size_t size() const;

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

private:
    mutable size_t rpos_{0};
};
}
