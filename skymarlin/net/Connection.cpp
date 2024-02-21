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

#include <skymarlin/net/Connection.hpp>

#include <skymarlin/net/MessageResolver.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::net
{
Connection::Connection(boost::asio::io_context& io_context, Socket&& socket)
    : io_context_(io_context), socket_(std::move(socket)) {}

Connection::~Connection()
{
    if (connected_) {
        SKYMARLIN_LOG_WARN("Connection destructing without being disconnected");
    }
}

void Connection::StartReceiveMessage()
{
    if (!connected_) return;

    co_spawn(io_context_, [this]()-> boost::asio::awaitable<void> {
        while (connected_) {
            auto message = co_await ReceiveMessage();
            if (!message) continue;

            HandleMessage(std::move(message));
        }
    }, boost::asio::detached);
}

void Connection::Disconnect()
{
    if (!connected_.exchange(false)) return;

    co_spawn(io_context_, [this]()-> boost::asio::awaitable<void> {
        auto [ec] = co_await socket_.async_shutdown(as_tuple(boost::asio::use_awaitable));
        if (ec) {
            SKYMARLIN_LOG_ERROR("Error on shutdown socket: {}", ec.what());
        }
    }, boost::asio::detached);
}

void Connection::SendMessage(std::shared_ptr<Message> message)
{
    if (!message || !connected_) return;

    send_queue_.Push(std::move(message));

    if (send_queue_processing_.exchange(true)) return;
    co_spawn(io_context_, SendMessageQueue(), boost::asio::detached);
}

boost::asio::awaitable<bool> Connection::Connect(Connection& connection, std::string_view host, uint16_t port)
{
    tcp::resolver resolver {connection.io_context_};

    SKYMARLIN_LOG_INFO("Trying to connect to {}:{}", host, port);
    const auto [ec, endpoints] = co_await resolver.async_resolve(host,
        std::format("{}", port), as_tuple(boost::asio::use_awaitable));
    if (ec) {
        SKYMARLIN_LOG_ERROR("Error on resolve: {}", ec.what());
        connection.connected_ = false;
        co_return false;
    }

    if (const auto [ec, endpoint] = co_await async_connect(connection.socket_.lowest_layer(), endpoints,
        as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on connect: {}", ec.what());
        connection.connected_ = false;
        co_return false;
    }

    if (const auto [ec] = co_await connection.socket_.async_handshake(boost::asio::ssl::stream_base::client,
        as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on handshake: {}", ec.what());
        connection.Disconnect();
        co_return false;
    }

    co_return true;
}

boost::asio::awaitable<void> Connection::SendMessageQueue()
{
    while (!send_queue_.empty()) {
        const auto message = send_queue_.Pop();

        //TODO: buffer pooling?
        std::vector<byte> buffer(MessageHeader::HeaderSize + message->size());
        MessageHeader header = message->header();

        MessageHeader::WriteHeader(buffer.data(), header);
        if (!message->Serialize(buffer.data() + MessageHeader::HeaderSize, header.size)) {
            SKYMARLIN_LOG_ERROR("Failed to serialize packet body");
            Disconnect();
            co_return;
        }

        if (const auto [ec, _] = co_await async_write(socket_,
            boost::asio::buffer(buffer), as_tuple(boost::asio::use_awaitable)); ec) {
            SKYMARLIN_LOG_ERROR("Error on sending packet: {}", ec.what());
            Disconnect();
            co_return;
        }
    }

    send_queue_processing_ = false;
}

void Connection::HandleMessage(std::unique_ptr<Message> message)
{
    auto& handler = MessageResolver::ResolveHandler(message->type());
    if (!handler) return;

    handler(std::move(message), shared_from_this());
}

boost::asio::awaitable<std::unique_ptr<Message>> Connection::ReceiveMessage()
{
    const std::optional<MessageHeader> header = co_await ReadMessageHeader();
    if (!header) {
        Disconnect();
        co_return nullptr;
    }

    std::unique_ptr<Message> message = MessageResolver::ResolveFactory(header->type);
    if (!message) {
        SKYMARLIN_LOG_ERROR("Invalid message type({})", header->type);
        Disconnect();
        co_return nullptr;
    }

    message = co_await ReadMessageBody(std::move(message), header->size);
    if (!message) {
        Disconnect();
        co_return nullptr;
    }

    co_return message;
}

boost::asio::awaitable<std::optional<MessageHeader>> Connection::ReadMessageHeader()
{
    if (const auto [ec, _] = co_await async_read(socket_,
        boost::asio::buffer(header_buffer_), as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet header: {}", ec.what());
        co_return std::nullopt;
    }

    co_return MessageHeader::ReadHeader(header_buffer_);
}

boost::asio::awaitable<std::unique_ptr<Message>> Connection::ReadMessageBody(std::unique_ptr<Message> message,
    const MessageSize size)
{
    //TODO: buffer pooling?
    std::vector<byte> buffer(size);

    if (const auto [ec, _] = co_await async_read(socket_,
        boost::asio::buffer(buffer), as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet body: {}", ec.what());
        co_return nullptr;
    }

    if (!message->Deserialize(buffer.data(), size)) {
        SKYMARLIN_LOG_ERROR("Failed to deserialize packet body");
        co_return nullptr;
    }

    co_return message;
}
}
