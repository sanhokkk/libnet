#pragma once

#include <boost/core/noncopyable.hpp>
#include <skymarlin/util/BitConverter.hpp>
#include <skymarlin/util/Memory.hpp>

#include <functional>

namespace skymarlin::net {
using MessageType = uint16_t;
using MessageSize = uint16_t;

using util::Memory;


struct MessageHeader {
    MessageType type{0};
    MessageSize size{0};

    static constexpr size_t HEADER_SIZE = sizeof(decltype(type)) + sizeof(decltype(size));

    static MessageHeader ReadHeader(const std::array<byte, HEADER_SIZE>& src);

    static void WriteHeader(std::array<byte, HEADER_SIZE>& dest, const MessageHeader &src);
};


class Message : boost::noncopyable {
public:
    Message(std::vector<byte> &&buffer, MessageHeader header)
        : buffer_(std::move(buffer)), header_(header) {}

    virtual ~Message() {
        //TODO: return `buffer_` for reuse
    }

    const MessageHeader &header() const { return header_; }
    std::vector<byte>& buffer() { return buffer_; }

protected:
    std::vector<byte> buffer_;

private:
    const MessageHeader header_;
};


inline MessageHeader MessageHeader::ReadHeader(const std::array<byte, HEADER_SIZE>& src) {
    using util::BitConverter;

    MessageHeader header{};
    size_t pos{0};

    header.type = BitConverter::Read<MessageType>(src.data() + pos);
    pos += sizeof(MessageType);

    header.size = BitConverter::Read<MessageSize>(src.data() + pos);
    pos += sizeof(MessageSize);

    return header;
}

inline void MessageHeader::WriteHeader(std::array<byte, HEADER_SIZE>& dest, const MessageHeader &src) {
    using util::BitConverter;

    size_t pos{0};

    BitConverter::Write<MessageSize>(src.size, dest.data() + pos);
    pos += sizeof(MessageSize);

    BitConverter::Write<MessageType>(src.type, dest.data() + pos);
    pos += sizeof(MessageType);
}
}
