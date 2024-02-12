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
using PacketCrypto = u8;

constexpr static size_t PACKET_HEADER_SIZE = sizeof(PacketLength) + sizeof(PacketType);

class Session;


struct PacketHeader
{
    PacketLength length {0};
    PacketType type {INVALID_PACKET_TYPE};
    PacketCrypto crypto {0};

    explicit operator bool() const { return type != INVALID_PACKET_TYPE; }

    static constexpr PacketType INVALID_PACKET_TYPE = 0;
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
};


inline PacketHeader Packet::ReadHeader(byte* src)
{
    PacketHeader header;
    size_t pos {0};

    header.length = utility::BitConverter::Convert<PacketLength>(src + pos);
    pos += sizeof(PacketLength);

    header.type = utility::BitConverter::Convert<PacketType>(src + pos);
    pos += sizeof(PacketType);

    header.crypto = utility::BitConverter::Convert<PacketCrypto>(src + pos);
    // pos += sizeof(PacketCrypto);

    return header;
}

inline void Packet::WriteHeader(byte* dest, const PacketHeader& src)
{
    size_t pos {0};

    utility::BitConverter::Convert<PacketLength>(src.length, dest + pos);
    pos += sizeof(PacketLength);

    utility::BitConverter::Convert<PacketType>(src.type, dest + pos);
    pos += sizeof(PacketType);

    utility::BitConverter::Convert<PacketCrypto>(src.crypto, dest + pos);
    // pos += sizeof(PacketCrypto);
}
}
