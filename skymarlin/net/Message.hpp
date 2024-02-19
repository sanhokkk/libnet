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

#include <boost/core/noncopyable.hpp>
#include <skymarlin/net/details/BitConverter.hpp>

#include <functional>
#include <memory>

namespace skymarlin::net
{
class Message;

using MessageType = uint16_t;
using MessageSize = uint16_t;
using MessageFactory = std::function<std::unique_ptr<Message>()>;


struct MessageHeader
{
    MessageType type {};
    MessageSize size {0};

    static constexpr size_t HeaderSize = sizeof(decltype(type)) + sizeof(decltype(size));

    static MessageHeader ReadHeader(byte* src);
    static void WriteHeader(byte* dest, const MessageHeader& src);
};


class Message : boost::noncopyable
{
public:
    explicit Message(MessageType type);
    virtual ~Message() = default;

    virtual bool Serialize(byte* dest, size_t size) const = 0;
    virtual bool Deserialize(const byte* src, size_t size) = 0;

    MessageType type() const { return type_; }
    virtual MessageSize size() const = 0;
    MessageHeader header() const { return {.type = type(), .size = size()}; }

    template <typename T> requires std::is_base_of_v<Message, T>
    static std::pair<MessageType, MessageFactory> Factory(MessageType type);

private:
    const MessageType type_;
};

inline Message::Message(const MessageType type)
    : type_(type) {}


inline MessageHeader MessageHeader::ReadHeader(byte* src)
{
    MessageHeader header;
    size_t pos {0};

    header.type = details::BitConverter::Convert<MessageType>(src + pos);
    pos += sizeof(MessageType);

    header.size = details::BitConverter::Convert<MessageSize>(src + pos);
    pos += sizeof(MessageSize);

    return header;
}

inline void MessageHeader::WriteHeader(byte* dest, const MessageHeader& src)
{
    size_t pos {0};

    details::BitConverter::Convert<MessageSize>(src.size, dest + pos);
    pos += sizeof(MessageSize);

    details::BitConverter::Convert<MessageType>(src.type, dest + pos);
    pos += sizeof(MessageType);
}

template <typename T> requires std::is_base_of_v<Message, T>
std::pair<MessageType, MessageFactory> Message::Factory(MessageType type)
{
    return {type, [] { return std::make_unique<T>(); }};
}
}
