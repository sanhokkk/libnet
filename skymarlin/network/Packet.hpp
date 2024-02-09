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
#include <skymarlin/utility/BitConverter.hpp>
#include <skymarlin/utility/TypeDefinitions.hpp>

namespace skymarlin::network
{
using PacketLength = u16; // Exclude header length
using PacketType = u8;
// Reserve u8 for crypto type

constexpr static size_t PACKET_HEADER_SIZE = sizeof(PacketLength) + sizeof(PacketType);

class Session;


struct PacketHeader
{
    PacketLength length {0};
    PacketType type {INVALID_PACKET_TYPE};
    u8 dummy {0};

    explicit operator bool() const { return type != INVALID_PACKET_TYPE; }

    static constexpr PacketType INVALID_PACKET_TYPE = 0;
};


class Packet : boost::noncopyable
{
public:
    virtual ~Packet() = default;

    virtual void Serialize(byte* dest) const = 0;
    virtual void Deserialize(byte* src) = 0;
    virtual void Handle(std::shared_ptr<Session> session) = 0;

    virtual PacketLength length() const = 0;
    virtual PacketHeader header() const = 0;

    static PacketHeader ReadHeader(byte* src);
    static void WriteHeader(byte* dest, const PacketHeader& header);
};


inline PacketHeader Packet::ReadHeader(byte* src)
{
    PacketHeader header;
    size_t pos {0};

    header.length = utility::BitConverter::Convert<PacketLength>(src + pos);
    pos += sizeof(PacketLength);

    header.type = utility::BitConverter::Convert<PacketType>(src + pos);
    pos += sizeof(PacketType);

    header.dummy = utility::BitConverter::Convert<u8>(src + pos);
    // pos += sizeof(u8);

    return header;
}

inline void Packet::WriteHeader(byte* dest, const PacketHeader& header)
{
    size_t pos {0};

    utility::BitConverter::Convert<PacketLength>(header.length, dest + pos);
    pos += sizeof(PacketLength);

    utility::BitConverter::Convert<PacketType>(header.type, dest + pos);
    pos += sizeof(PacketType);

    utility::BitConverter::Convert<PacketType>(header.dummy, dest + pos);
    // pos += sizeof(u8);
}
}
