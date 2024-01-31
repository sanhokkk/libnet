#include <skymarlin/utility/ConstByteBuffer.hpp>

#include <cmath>

#include <skymarlin/utility/ByteBufferExceptions.hpp>

namespace skymarlin::utility
{
ConstByteBuffer::ConstByteBuffer(byte* buffer, const size_t size)
    : data_(buffer), size_(size) {}

ConstByteBuffer::ConstByteBuffer(ConstByteBuffer&& x) noexcept
    : data_(x.data_), size_(x.size_), rpos_(x.rpos_)
{
    x.data_ = nullptr;
    x.size_ = 0;
    x.rpos_ = 0;
}


template <NumericType T>
T ConstByteBuffer::Read() const
{
    T value = BitConverter::Convert<T>(data_ + rpos_);
    rpos_ += sizeof(T);
    return value;
}

template <typename T> requires std::same_as<T, std::string>
T ConstByteBuffer::Read() const
{
    const size_t length = Read<u16>();
    if (rpos_ + length > size_) {
        throw ByteBufferInvalidValueException(true, std::to_string(length), "string length");
    }

    std::string value;
    const size_t end_pos = rpos_ + length;
    while (rpos_ < end_pos) {
        // TODO: encoding
        value += Read<char>();
    }

    return value;
}

void ConstByteBuffer::Skip(const size_t n) const
{
    if (rpos_ + n > size_) {
        throw ByteBufferPositionException(true, rpos_, n, size_);
    }

    rpos_ += n;
}

const byte& ConstByteBuffer::operator[](const size_t pos) const
{
    if (pos > size_) {
        throw ByteBufferPositionException(true, pos, sizeof(byte), size_);
    }

    return data_[pos];
}

ConstByteBuffer& ConstByteBuffer::operator>>(bool& value) const
{
    value = Read<byte>() > 0;
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(byte& value) const
{
    value = Read<byte>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(u16& value) const
{
    value = Read<u16>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(u32& value) const
{
    value = Read<u32>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(u64& value) const
{
    value = Read<u64>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(i8& value) const
{
    value = Read<i8>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(i16& value) const
{
    value = Read<i16>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(i32& value) const
{
    value = Read<i32>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(i64& value) const
{
    value = Read<i64>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(f32& value) const
{
    value = Read<f32>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(f64& value) const
{
    value = Read<f64>();
    return *const_cast<ConstByteBuffer*>(this);
}

ConstByteBuffer& ConstByteBuffer::operator>>(std::string& value) const
{
    value = Read<std::string>();

    return *const_cast<ConstByteBuffer*>(this);
}
}
