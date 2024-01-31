#pragma once

#include <skymarlin/network/Packet.hpp>
#include <skymarlin/protocol/chat/proto/ChatMessage.pb.h>

namespace skymarlin::protocol::chat {
class ChatMessagePacket final : public network::Packet
{
public:
    ChatMessagePacket() = default;

    ChatMessagePacket(const u32 client_id, std::string_view message)
    {
        chat_message_.set_client_id(client_id);
        chat_message_.set_message(message);
    }

    void Serialize(boost::asio::mutable_buffer& buffer) const override
    {
        // Packet::WriteHeader(buffer.data(), )
        // if (!chat_message_.SerializeToArray())
    }

    void Deserialize(const boost::asio::mutable_buffer& buffer) override
    {
        if (!chat_message_.ParseFromArray(buffer.data(), buffer.size())) {
            //TODO: Error
        }
    }

    void Handle(std::shared_ptr<network::Session> session) override {}

    size_t Length() const override { return chat_message_.ByteSizeLong(); }

private:
    ChatMessage chat_message_{};
};
}
