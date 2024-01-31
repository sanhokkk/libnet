#pragma once

#include <skymarlin/network/Packet.hpp>
#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/protocol/chat/ChatMessagePacket.hpp>

namespace skymarlin::protocol::chat
{
using network::PacketLength;
using network::PacketType;
using network::PacketCreator;

enum class ChatPacketType : PacketType
{
    ChatMessagePacket = 0x01,
};

inline std::vector<std::pair<PacketType, PacketCreator>> GetChatPacketCreators()
{
    return {
        {
            static_cast<PacketType>(ChatPacketType::ChatMessagePacket),
            [] { return std::make_shared<ChatMessagePacket>(); }
        },
    };
}
}
