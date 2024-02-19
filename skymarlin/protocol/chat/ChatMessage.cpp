#include <skymarlin/protocol/chat/ChatMessage.hpp>

#include <skymarlin/protocol/chat/ChatMessageTypes.hpp>

namespace skymarlin::protocol::chat
{
ChatMessage::ChatMessage()
    : Message(static_cast<MessageType>(ChatMessageTypes::ChatMessage)) {}

ChatMessage::ChatMessage(std::string_view user_id, std::string_view message)
    : Message(static_cast<MessageType>(ChatMessageTypes::ChatMessage))
{
    chat_.set_user_id(user_id);
    chat_.set_message(message);
}

bool ChatMessage::Serialize(byte* dest, const size_t size) const
{
    return chat_.SerializeToArray(dest, static_cast<int>(size));
}

bool ChatMessage::Deserialize(const byte* src, const size_t size)
{
    return chat_.ParseFromArray(src, static_cast<int>(size));
}

net::MessageSize ChatMessage::size() const
{
    return chat_.ByteSizeLong();
}
}
