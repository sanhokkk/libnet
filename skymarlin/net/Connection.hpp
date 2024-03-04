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

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/net/Message.hpp>
#include <skymarlin/util/Queue.hpp>

namespace skymarlin::net
{
using boost::asio::ip::tcp;
using Socket = boost::asio::ssl::stream<tcp::socket>;

class Connection final : public std::enable_shared_from_this<Connection>, boost::noncopyable
{
public:
    Connection(boost::asio::io_context& io_context, Socket&& socket);
    ~Connection();

    void Disconnect();
    void StartReceiveMessage();
    void SendMessage(std::shared_ptr<Message> message);

    static boost::asio::awaitable<bool> Connect(Connection& connection, std::string_view host, uint16_t port);

	bool connected() const { return connected_; }
    tcp::endpoint local_endpoint() const { return socket_.lowest_layer().local_endpoint(); }
    tcp::endpoint remote_endpoint() const { return socket_.lowest_layer().remote_endpoint(); }

private:
    boost::asio::awaitable<std::unique_ptr<Message>> ReceiveMessage();
    boost::asio::awaitable<std::optional<MessageHeader>> ReadMessageHeader();
    boost::asio::awaitable<std::optional<std::vector<byte>>> ReadMessageBody(MessageHeader header);
    boost::asio::awaitable<void> SendMessageQueue();
    void HandleMessage(std::unique_ptr<Message> message);

    boost::asio::io_context& io_context_;
    Socket socket_;
    std::atomic<bool> connected_ {true};

    util::ConcurrentQueue<std::shared_ptr<Message>> send_queue_ {};
    std::atomic<bool> send_queue_processing_ {false};
};
}
