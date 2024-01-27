#pragma once

#include <skymarlin/network/packet/Packet.hpp>
#include <skymarlin/network/packet/PacketResolver.hpp>
#include <skymarlin/protocol/chat/ChatMessagePacket.hpp>

namespace skymarlin::chat::packets {
using network::PacketLength;
using network::PacketType;
using network::PacketCreator;

enum class ChatPackets : PacketType {
    ChatMessagePacketType = 0x01,
};

static const std::vector<std::pair<PacketType, PacketCreator>> CHAT_PACKET_CREATORS = {
    {ChatPackets::ChatMessagePacketType, [] { return std::make_unique<ChatMessagePacket>(); }}
};
}
