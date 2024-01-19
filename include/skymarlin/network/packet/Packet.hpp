#pragma once

#include <skymarlin/TypeDefinitions.hpp>
#include <skymarlin/network/packet/MutableByteBuffer.hpp>

namespace skymarlin::network::packet {
constexpr static size_t PACKET_HEADER_SIZE = sizeof(u32);
constexpr static size_t PACKET_LENGTH_SIZE = sizeof(u16);
constexpr static size_t PACKET_TYPE_SIZE = sizeof(u8);

class Packet {
public:
    explicit Packet(const ConstByteBuffer& header) {
        header >> length_ >> type_;
    }

    virtual ~Packet() = default;

    size_t length() const {
        return length_;
    }

    virtual void Serialize(MutableByteBuffer& buffer) = 0;

    virtual void Deserialize(const ConstByteBuffer& buffer) = 0;

private:
    u16 length_{};
    u8 type_{};
};
}
