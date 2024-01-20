#pragma once

#include <skymarlin/network/TypeDefinitions.hpp>
#include <skymarlin/network/packet/MutableByteBuffer.hpp>

namespace skymarlin::network::packet {
using PacketLength = u16;
using PacketType = u8;

constexpr static size_t PACKET_HEADER_SIZE = sizeof(u32);
// constexpr static size_t PACKET_LENGTH_SIZE = sizeof(PacketLength);
// constexpr static size_t PACKET_TYPE_SIZE = sizeof(PacketType);


class Packet : boost::noncopyable {
public:
    Packet(const PacketLength length, const PacketType type) : length_(length), type_(type) {
    }

    virtual ~Packet() = default;

    size_t length() const {
        return length_;
    }

    virtual void Serialize(MutableByteBuffer& buffer) = 0;

    virtual void Deserialize(const ConstByteBuffer& buffer) = 0;

protected:
    PacketLength length_;
    PacketType type_;
};
}
