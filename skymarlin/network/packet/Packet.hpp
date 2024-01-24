#pragma once

#include <skymarlin/network/TypeDefinitions.hpp>
#include <skymarlin/network/utility/MutableByteBuffer.hpp>

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

    virtual void Serialize(boost::asio::mutable_buffer& buffer) const = 0;

    virtual void Deserialize(boost::asio::mutable_buffer& buffer)= 0;

    virtual void Handle(std::shared_ptr<Session> session) = 0;
};
}
