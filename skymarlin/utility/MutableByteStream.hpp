/*
 * MIT License
 *
 * Copyright (c) 2024 skymarlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <skymarlin/utility/BitConverter.hpp>
#include <skymarlin/utility/ByteStreamExceptions.hpp>
#include <skymarlin/utility/ConstByteStream.hpp>

namespace skymarlin::utility
{
class MutableByteStream : public ConstByteStream
{
public:
    MutableByteStream(byte* buffer, size_t size);

    MutableByteStream(MutableByteStream&& x) noexcept;

    void Clear() const;

    template <NumericType T>
    void Append(T value);

    template <typename T> requires std::same_as<T, std::string_view>
    void Append(T value);

    byte* data() const { return data_; }

    byte& operator[](size_t pos) const;

    MutableByteStream& operator<<(bool value);

    MutableByteStream& operator<<(byte value);

    MutableByteStream& operator<<(u16 value);

    MutableByteStream& operator<<(u32 value);

    MutableByteStream& operator<<(u64 value);

    MutableByteStream& operator<<(i8 value);

    MutableByteStream& operator<<(i16 value);

    MutableByteStream& operator<<(i32 value);

    MutableByteStream& operator<<(i64 value);

    MutableByteStream& operator<<(f32 value);

    MutableByteStream& operator<<(f64 value);

    MutableByteStream& operator<<(std::string_view value);

    MutableByteStream& operator<<(const std::string& value);

private:
    mutable size_t wpos_{0};
};

inline MutableByteStream::MutableByteStream(byte* buffer, const size_t size)
    : ConstByteStream(buffer, size) {}

inline MutableByteStream::MutableByteStream(MutableByteStream&& x) noexcept
    : ConstByteStream(x.data_, x.size_), wpos_(x.wpos_)
{
    rpos_ = x.rpos_;

    x.data_ = nullptr;
    x.size_ = 0;
    x.rpos_ = 0;
    x.wpos_ = 0;
}

inline void MutableByteStream::Clear() const
{
    std::memset(data_, 0, size_);
}

template <NumericType T>
void MutableByteStream::Append(const T value)
{
    if (wpos_ + sizeof(T) > size_) {
        throw ByteBufferPositionException(false, wpos_, sizeof(T), size_);
    }

    BitConverter::Convert<T>(value, data_ + wpos_);
    wpos_ += sizeof(T);
}

template <typename T> requires std::same_as<T, std::string_view>
void MutableByteStream::Append(const T value)
{
    //TODO: Convert via BitConverter
    const size_t length = value.length();
    if (wpos_ + sizeof(STRING_HEADER_SIZE) + length > size_ || length > std::numeric_limits<StringLengthType>::max()) {
        throw ByteBufferInvalidValueException(false, std::to_string(length), "string length");
    }
    Append<u16>(length);

    for (const char& c : value) {
        // TODO: encoding
        Append<char>(c);
    }
}

inline byte& MutableByteStream::operator[](const size_t pos) const
{
    if (pos > size_) { throw ByteBufferPositionException(true, pos, sizeof(byte), size_); }

    return data_[pos];
}

inline MutableByteStream& MutableByteStream::operator<<(const bool value)
{
    Append<byte>(value ? 1 : 0);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const byte value)
{
    Append<byte>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const u16 value)
{
    Append<u16>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const u32 value)
{
    Append<u32>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const u64 value)
{
    Append<u64>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const i8 value)
{
    Append<i8>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const i16 value)
{
    Append<i16>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const i32 value)
{
    Append<i32>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const i64 value)
{
    Append<i64>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const f32 value)
{
    Append<f32>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const f64 value)
{
    Append<f64>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const std::string_view value)
{
    Append<std::string_view>(value);
    return *this;
}

inline MutableByteStream& MutableByteStream::operator<<(const std::string& value)
{
    Append<std::string_view>(value);
    return *this;
}
}
