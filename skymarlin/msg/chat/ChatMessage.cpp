#include <skymarlin/protocol/chat/ChatMessage.hpp>

#include <skymarlin/protocol/chat/ChatMessageTypes.hpp>

namespace skymarlin::protocol::chat
{
ChatMessage::ChatMessage()
    : header_() {}
    // : Message(static_cast<MessageType>(ChatMessageTypes::ChatMessage)) {}

ChatMessage::ChatMessage(std::string_view user_id, std::string_view message)
    : Message(static_cast<MessageType>(ChatMessageTypes::ChatMessage))
{
    message_source_.set_user_id(user_id);
    message_source_.set_message(message);
}
}
