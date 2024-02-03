#pragma once

#include <format>

namespace skymarlin::utility
{
using namespace std::literals::string_view_literals;

class ByteBufferException : public std::exception
{
public:
    ~ByteBufferException() override = default;

    const char* what() const noexcept override { return message_.c_str(); }

protected:
    std::string& message() noexcept { return message_; }

private:
    std::string message_;
};

class ByteBufferPositionException final : public ByteBufferException
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

class ByteBufferInvalidValueException final : public ByteBufferException
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
