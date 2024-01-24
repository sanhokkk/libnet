#include <skymarlin/network/packet/MutableByteBuffer.hpp>

#include <cmath>
#include <limits>

#include <skymarlin/network/packet/ByteBufferExceptions.hpp>

namespace skymarlin::network::packet {
MutableByteBuffer::MutableByteBuffer(const boost::asio::mutable_buffer& buffer)
    : ConstByteBuffer(boost::asio::buffer_cast<byte*>(buffer), buffer.size()) {
}

MutableByteBuffer::MutableByteBuffer(byte* buffer, const size_t size)
    : ConstByteBuffer(buffer, size) {
}

MutableByteBuffer::MutableByteBuffer(MutableByteBuffer&& x) noexcept
    : ConstByteBuffer(x.data_, x.size_), wpos_(x.wpos_) {
    rpos_ = x.rpos_;

    x.data_ = nullptr;
    x.size_ = 0;
    x.rpos_ = 0;
    x.wpos_ = 0;
}

void MutableByteBuffer::Clear() {
    std::memset(data_, 0, size_);
}

template<NumericType T>
void MutableByteBuffer::Append(T value) {
    Put<T>(value, wpos_);
    wpos_ += sizeof(T);
}

template<typename T> requires std::same_as<T, std::string_view>
void MutableByteBuffer::Append(T value) {
    const size_t length = value.length();
    if (wpos_ + length > size_ || length > std::numeric_limits<u16>::max()) {
        throw ByteBufferInvalidValueException("string length", std::to_string(length));
    }
    Append<u16>(length);

    for (const char& c: value) {
        // TODO: encoding
        Append<char>(c);
    }
}

template<NumericType T>
void MutableByteBuffer::Put(T value, const size_t pos) {
    if (pos + sizeof(T) > size_) {
        throw ByteBufferPositionException(false, pos, sizeof(T), size_);
    }

    if constexpr (std::is_same_v<T, f32> || std::is_same_v<T, f64>) {
        if (std::isinf(value)) {
            throw ByteBufferInvalidValueException("infinity", "f32 || f64");
        }
    }

    std::memset(data_ + pos, value, sizeof(T));
}

byte* MutableByteBuffer::data() const {
    return data_;
}

byte& MutableByteBuffer::operator[](const size_t pos) const {
    if (pos > size_) {
        throw ByteBufferPositionException(true, pos, sizeof(byte), size_);
    }

    return data_[pos];
}

MutableByteBuffer& MutableByteBuffer::operator<<(const bool value) {
    Append<byte>(value > 0);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const byte value) {
    Append<byte>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const u16 value) {
    Append<u16>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const u32 value) {
    Append<u32>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const u64 value) {
    Append<u32>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const i8 value) {
    Append<i8>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const i16 value) {
    Append<i16>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const i32 value) {
    Append<i32>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const i64 value) {
    Append<i64>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const f32 value) {
    Append<f32>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const f64 value) {
    Append<f64>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const std::string_view value) {
    Append<std::string_view>(value);
    return *this;
}

MutableByteBuffer& MutableByteBuffer::operator<<(const std::string& value) {
    Append<std::string_view>(value);
    return *this;
}
}
