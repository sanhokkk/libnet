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

#include <skymarlin/protocol/chat/ChatMessagePacket.hpp>

#include <skymarlin/network/Session.hpp>
#include <skymarlin/protocol/chat/ChatPackets.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::protocol::chat
{
ChatMessagePacket::ChatMessagePacket(const u32 client_id, std::string_view message)
{
    chat_message_.set_client_id(client_id);
    chat_message_.set_message(message);
}

bool ChatMessagePacket::Serialize(byte* dest, const size_t size) const
{
    return chat_message_.SerializeToArray(dest, static_cast<int>(size));
}

bool ChatMessagePacket::Deserialize(const byte* src, const size_t size)
{
    return chat_message_.ParseFromArray(src, static_cast<int>(size));
}

void ChatMessagePacket::Handle(const std::shared_ptr<network::Session> session)
{
    SKYMARLIN_LOG_INFO("[Message] {} : {}", session->id(), chat_message_.message());
    //TODO: Client doesn't need to broadcast.
    session->BroadcastPacket(shared_from_this());
}

PacketLength ChatMessagePacket::length() const { return chat_message_.ByteSizeLong(); }

network::PacketHeader ChatMessagePacket::header() const
{
    return {.length = length(), .protocol = static_cast<PacketProtocol>(ChatPacketProtocol::ChatMessage)};
}
}
