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

#include <format>

namespace skymarlin::utility
{
using namespace std::literals::string_view_literals;

class ByteStreamException : public std::exception
{
public:
    ~ByteStreamException() override = default;

    const char* what() const noexcept override { return message_.c_str(); }

protected:
    std::string& message() noexcept { return message_; }

private:
    std::string message_;
};

class ByteBufferPositionException final : public ByteStreamException
{
public:
    ByteBufferPositionException(const bool read, const size_t pos, const size_t value_size,
        const size_t buffer_size)
    {
        message().assign(
            std::format("Attempted to {} {} bytes in ByteBuffer; pos: {}, size: {}"sv,
                (read ? "read"sv : "write"sv),
                value_size,
                pos,
                buffer_size));
    }

    ~ByteBufferPositionException() override = default;
};

class ByteBufferInvalidValueException final : public ByteStreamException
{
public:
    ByteBufferInvalidValueException(const bool read, const std::string_view value,
        const std::string_view value_type)
    {
        message().assign(
            std::format("Invalid {} of ({}){} on ByteBuffer"sv,
                (read ? "read"sv : "write"sv),
                value_type,
                value));
    }
};
};
