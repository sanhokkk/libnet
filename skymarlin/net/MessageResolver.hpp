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

#include <skymarlin/net/Connection.hpp>
#include <skymarlin/net/Message.hpp>

#include <functional>
#include <unordered_map>

namespace skymarlin::net
{
using MessageFactory = std::function<std::unique_ptr<Message>(std::vector<byte>&&, MessageHeader)>;
using MessageHandler = std::function<void(std::unique_ptr<Message>, std::shared_ptr<Connection>)>;

class MessageResolver final
{
public:
    static void RegisterFactories(std::vector<std::pair<MessageType, MessageFactory>>&& factories);
    static void RegisterHandlers(std::vector<std::pair<MessageType, MessageHandler>>&& handlers);
    static const MessageFactory& ResolveFactory(MessageType type);
    static const MessageHandler& ResolveHandler(MessageType type);

private:
    inline static std::unordered_map<MessageType, MessageFactory> factory_map_ {};
    inline static std::unordered_map<MessageType, MessageHandler> handler_map_ {};

    inline static const MessageFactory empty_factory_ {};
    inline static const MessageHandler empty_handler_ {};
};


inline void MessageResolver::RegisterFactories(std::vector<std::pair<MessageType, MessageFactory>>&& factories)
{
    for (auto& [type, factory] : factories) {
        factory_map_[type] = std::move(factory);
    }
}

inline void MessageResolver::RegisterHandlers(std::vector<std::pair<MessageType, MessageHandler>>&& handlers)
{
    for (auto& [type, handler] : handlers) {
        handler_map_[type] = std::move(handler);
    }
}


inline const MessageFactory& MessageResolver::ResolveFactory(const MessageType type)
{
    if (!factory_map_.contains(type)) {
        return empty_factory_;
    }
    return factory_map_[type];
}

inline const MessageHandler& MessageResolver::ResolveHandler(const MessageType type)
{
    if (!handler_map_.contains(type)) {
        return empty_handler_;
    }
    return handler_map_[type];
}

}
