#pragma once

#include <boost/core/noncopyable.hpp>
#include <skymarlin/util/BitConverter.hpp>

#include <limits>

namespace skymarlin::net {
using MessageType = uint16_t;
using MessageSize = uint16_t;


struct MessageHeader {
    const MessageType type;
    const MessageSize size;

    static constexpr size_t HEADER_SIZE = sizeof(decltype(type)) + sizeof(decltype(size));

    static MessageHeader ReadHeader(std::span<byte, HEADER_SIZE> src);
    static void WriteHeader(std::span<byte, HEADER_SIZE> dest, const MessageHeader& src);
};


class Message final : boost::noncopyable {
public:
    Message(std::vector<byte>&& buffer, const MessageHeader header)
        : buffer_(std::move(buffer)), header_(header) {}

    const MessageHeader& header() const { return header_; }
    std::span<const byte> buffer() const { return {buffer_}; }

    static constexpr size_t MAX_SIZE = std::numeric_limits<MessageSize>::max();

private:
    std::vector<byte> buffer_;
    const MessageHeader header_;
};


inline MessageHeader MessageHeader::ReadHeader(const std::span<byte, HEADER_SIZE> src) {
    size_t pos {0};

    auto type = util::BitConverter::Read<MessageType>(src.data() + pos);
    pos += sizeof(MessageType);

    auto size = util::BitConverter::Read<MessageSize>(src.data() + pos);
    pos += sizeof(MessageSize);

    return {.type = type, .size = size};
}

inline void MessageHeader::WriteHeader(std::span<byte, HEADER_SIZE> dest, const MessageHeader& src) {
    size_t pos {0};

    util::BitConverter::Write<MessageSize>(src.size, dest.data() + pos);
    pos += sizeof(MessageSize);

    util::BitConverter::Write<MessageType>(src.type, dest.data() + pos);
    pos += sizeof(MessageType);
}
}
