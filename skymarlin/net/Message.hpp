#pragma once

#include <format>

namespace skymarlin::net
{
class Message;

using MessageType = uint16_t;
using MessageSize = uint16_t;
using MessageFactory = std::function<std::shared_ptr<Message>()>;

struct MessageHeader
{
    MessageType type {};
    MessageSize size {0};

    static constexpr size_t HeaderSize = sizeof(decltype(type)) + sizeof(decltype(size));

    friend std::ostream& operator<<(std::ostream& os, const MessageHeader& header)
    {
        os << std::format("(MessageHeader) type: {}, size: {}", header.type, header.size);
        return os;
    }
};

class Message
{
public:
    void Serialize();
    void Deserialize();
    size_t Size();

private:
    MessageHeader header_ {};
};
}
