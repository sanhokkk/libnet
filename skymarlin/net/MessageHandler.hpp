#pragma once

#include <skymarlin/net/IConnection.hpp>
#include <skymarlin/net/Message.hpp>

#include <functional>

namespace skymarlin::net {
using MessageHandlerType = std::function<void(std::unique_ptr<Message>, std::shared_ptr<IConnection>)>;

class MessageHandler final {
public:
    static void InitHandler(MessageHandlerType&& handler) {
        handler_ = std::move(handler);
    }

    static void Handle(std::unique_ptr<Message> message, std::shared_ptr<IConnection> connection) {
        handler_(std::move(message), std::move(connection));
    }

private:
    inline static MessageHandlerType handler_ {};
};
}
