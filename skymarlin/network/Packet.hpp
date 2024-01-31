#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/utility/BitConverter.hpp>
#include <skymarlin/utility/TypeDefinitions.hpp>

namespace skymarlin::network
{
using PacketLength = u16; // Exclude header length
using PacketType = u8;
// Reserve u8 for crypto type

constexpr static size_t PACKET_HEADER_SIZE = sizeof(PacketLength) + sizeof(PacketType);

class Session;

class Packet : boost::noncopyable
{
public:
    Packet() = default;

    virtual ~Packet() = default;

    virtual void Serialize(boost::asio::mutable_buffer& buffer) const = 0;
    virtual void Deserialize(const boost::asio::mutable_buffer& buffer) = 0;
    virtual void Handle(std::shared_ptr<Session> session) = 0;
    virtual size_t Length() const = 0;

    static void ReadHeader(byte* buffer, PacketLength& length, PacketType& type)
    {
        size_t pos{0};

        length = utility::BitConverter::Convert<PacketLength>(buffer + pos);
        pos += sizeof(PacketLength);

        type = utility::BitConverter::Convert<PacketType>(buffer + pos);
        pos += sizeof(PacketType);
    }

    static void WriteHeader(byte* buffer, const PacketLength length, const PacketType type)
    {
        size_t pos{0};

        utility::BitConverter::Convert<PacketLength>(length, buffer + pos);
        pos += sizeof(PacketLength);

        utility::BitConverter::Convert<PacketType>(type, buffer + pos);
        pos += sizeof(PacketType);
    }
};
}
