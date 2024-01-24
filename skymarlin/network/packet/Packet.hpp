#pragma once

#include <skymarlin/network/TypeDefinitions.hpp>
#include <skymarlin/network/packet/MutableByteBuffer.hpp>

namespace skymarlin::network {
class Session;
}

namespace skymarlin::network::packet {
using PacketLength = u16;
using PacketType = u16;

constexpr static size_t PACKET_HEADER_SIZE = sizeof(u32);


class Packet : boost::noncopyable {
public:
    Packet() = default;

    virtual ~Packet() = default;

    virtual void Serialize(byte* buffer) const = 0;

    virtual void Deserialize(const byte* buffer)= 0;

    virtual void Handle(std::shared_ptr<Session> session) = 0;
};
}
