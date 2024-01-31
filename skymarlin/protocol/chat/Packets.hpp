#pragma once

#include <skymarlin/network/packet/Packet.hpp>
#include <skymarlin/network/packet/PacketResolver.hpp>
#include <skymarlin/protocol/chat/ChatMessagePacket.hpp>

namespace skymarlin::protocol::chat {
using network::PacketLength;
using network::PacketType;
using network::PacketCreator;

enum class ChatPacketType : PacketType {
    ChatMessagePacket = 0x01,
};

std::vector<std::pair<PacketType, PacketCreator>> GetChatPacketCreators() {
    return std::vector {
        {ChatPacketType::ChatMessagePacket, [] { return std::make_shared<ChatMessagePacket>(); },
    }
}
}
