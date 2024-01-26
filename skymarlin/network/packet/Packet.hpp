#pragma once

#include <skymarlin/utility/TypeDefinitions.hpp>

namespace skymarlin::network {
class Session;
}

namespace skymarlin::network::packet {
using PacketLength = u16; // Exclude header length
using PacketType = u8;
// Reserve u8 for crypto type

constexpr static size_t PACKET_HEADER_SIZE = sizeof(PacketLength) + sizeof(PacketType);


class Packet : boost::noncopyable {
public:
    Packet() = default;

    virtual ~Packet() = default;

    virtual void Serialize(boost::asio::mutable_buffer& buffer) const = 0;

    virtual void Deserialize(const boost::asio::mutable_buffer& buffer) = 0;

    virtual void Handle(std::shared_ptr<Session> session) = 0;

    virtual size_t Length() const = 0;

    static void WriteHeader(byte* buffer, const PacketLength length, const PacketType type) {
        //TODO: Check buffer has at least header size
        size_t pos = 0;

        std::memset(buffer + pos, length, sizeof(PacketLength));
        pos += sizeof(PacketLength);

        std::memset(buffer + pos, type, sizeof(PacketType));
        pos += sizeof(PacketType);
    }
};
}
