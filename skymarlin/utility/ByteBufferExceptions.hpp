#pragma once

#include <sstream>

namespace skymarlin::utility {
class ByteBufferException : public std::exception {
public:
    ~ByteBufferException() override = default;

    const char* what() const noexcept override { return message_.c_str(); }

protected:
    std::string& message() noexcept { return message_; }

private:
    std::string message_;
};

class ByteBufferPositionException final : public ByteBufferException {
public:
    ByteBufferPositionException(const bool read, const size_t pos, const size_t value_size, const size_t buffer_size) {
        std::ostringstream ss;

        //TODO: Use std::format
        ss << "Attempted to " << (read ? "read " : "write ") << value_size << " bytes in ByteBuffer (pos: " << pos <<
                " size: " << buffer_size << " bytes )";

        message().assign(ss.str());
    }

    ~ByteBufferPositionException() override = default;
};

class ByteBufferInvalidValueException final : public ByteBufferException {
public:
    ByteBufferInvalidValueException(const bool read, const std::string_view value, const std::string_view value_type) {
        std::ostringstream ss;

        ss << "Invalid " << (read ? "read" : "write") << " on " << value_type << ": " << value << " in ByteBuffer";

        message().assign(ss.str());
    }
};
};
