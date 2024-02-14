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

#include <skymarlin/network/Packet.hpp>
#include <skymarlin/protocol/chat/proto/ChatMessage.pb.h>

namespace skymarlin::protocol::chat
{
class ChatMessagePacket final : public network::Packet
{
public:
    ChatMessagePacket() = default;
    ChatMessagePacket(uint32_t client_id, std::string_view message);
    ~ChatMessagePacket() override = default;

    bool Serialize(byte* dest, size_t size) const override;
    bool Deserialize(const byte* src, size_t size) override;
    void Handle(std::shared_ptr<network::Session> session) override;

    network::PacketLength length() const override;
    network::PacketHeader header() const override;

private:
    ChatMessage chat_message_ {};
};
}
