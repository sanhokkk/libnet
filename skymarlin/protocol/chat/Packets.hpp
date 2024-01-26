#pragma once

#include <skymarlin/network/packet/Packet.hpp>
#include <skymarlin/network/packet/PacketResolver.hpp>
#include <skymarlin/protocol/chat/ChatMessagePacket.hpp>

namespace skymarlin::chat::packets {
using network::packet::PacketLength;
using network::packet::PacketType;
using network::packet::PacketCreator;

enum ChatServerPackets : PacketType {
    CHAT_MESSAGE_PACKET_TYPE = 0x01,
};

static const std::vector<std::pair<PacketType, PacketCreator>> CHAT_PACKET_CREATORS = {
    {CHAT_MESSAGE_PACKET_TYPE, [] { return std::make_unique<ChatMessagePacket>(); }}
};
}
