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

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/net/details/BitConverter.hpp>

namespace skymarlin
{
using byte = uint8_t;
}

namespace skymarlin::net
{
class Session;
class Packet;

using PacketLength = uint16_t; // Exclude header length
using PacketProtocol = uint16_t;
using PacketFactory = std::function<std::shared_ptr<Packet>()>;

struct PacketHeader
{
    PacketLength length {0};
    PacketProtocol protocol {InvalidProtocol};

    explicit operator bool() const { return protocol != InvalidProtocol; }

    static constexpr size_t Size = sizeof(PacketLength) + sizeof(PacketProtocol);
    static constexpr PacketProtocol InvalidProtocol = 0;
};


class Packet : public std::enable_shared_from_this<Packet>, boost::noncopyable
{
public:
    virtual ~Packet() = default;

    virtual bool Serialize(byte* dest, size_t size) const = 0;
    virtual bool Deserialize(const byte* src, size_t size) = 0;
    virtual void Handle(std::shared_ptr<Session> session) = 0;

    virtual PacketLength length() const = 0;
    virtual PacketHeader header() const = 0;

    static PacketHeader ReadHeader(byte* src);
    static void WriteHeader(byte* dest, const PacketHeader& src);

    template <typename T> requires std::is_base_of_v<Packet, T>
    static std::pair<PacketProtocol, PacketFactory> MakePacketFactory(PacketProtocol type);
};


inline PacketHeader Packet::ReadHeader(byte* src)
{
    PacketHeader header;
    size_t pos {0};

    header.length = details::BitConverter::Convert<PacketLength>(src + pos);
    pos += sizeof(PacketLength);

    header.protocol = details::BitConverter::Convert<PacketProtocol>(src + pos);
    pos += sizeof(PacketProtocol);

    return header;
}

inline void Packet::WriteHeader(byte* dest, const PacketHeader& src)
{
    size_t pos {0};

    details::BitConverter::Convert<PacketLength>(src.length, dest + pos);
    pos += sizeof(PacketLength);

    details::BitConverter::Convert<PacketProtocol>(src.protocol, dest + pos);
    pos += sizeof(PacketProtocol);
}

template <typename PacketType> requires std::is_base_of_v<Packet, PacketType>
std::pair<PacketProtocol, PacketFactory> Packet::MakePacketFactory(PacketProtocol type)
{
    return {type, [] { return std::make_shared<PacketType>(); }};
}
}
