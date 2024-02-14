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

#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Packet.hpp>
#include <skymarlin/thread/Queue.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;
using Socket = boost::asio::ssl::stream<tcp::socket>;
using SessionFactory = std::function<std::shared_ptr<Session>(boost::asio::io_context&, Socket&&)>;
using SessionId = uint64_t;

class Session : public std::enable_shared_from_this<Session>, boost::noncopyable
{
public:
    Session(boost::asio::io_context& io_context, Socket&& socket);
    virtual ~Session();

    void Open();
    boost::asio::awaitable<void> Close();
    void SendPacket(std::shared_ptr<Packet> packet);
    void BroadcastPacket(std::shared_ptr<Packet> packet);

    SessionId id() const { return id_; }
    void set_id(const SessionId id) { id_ = id; }
    bool open() const { return !closed_ && !closing_; };
    tcp::endpoint local_endpoint() const { return socket_.lowest_layer().local_endpoint(); }
    tcp::endpoint remote_endpoint() const { return socket_.lowest_layer().remote_endpoint(); }

    template<typename SessionType> requires std::is_base_of_v<Session, SessionType>
    static SessionFactory MakeSessionFactory();

protected:
    virtual void OnOpen() = 0;
    virtual void OnClose() = 0;

    boost::asio::io_context& io_context_;

private:
    boost::asio::awaitable<std::shared_ptr<Packet>> ReceivePacket();
    boost::asio::awaitable<PacketHeader> ReadPacketHeader();
    boost::asio::awaitable<std::shared_ptr<Packet>> ReadPacketBody(std::shared_ptr<Packet> packet, PacketLength length);
    boost::asio::awaitable<void> SendPacketQueue();

    Socket socket_;
    byte header_buffer_[PACKET_HEADER_SIZE] {};
    thread::ConcurrentQueue<std::shared_ptr<Packet>> send_queue_ {};
    std::atomic<bool> send_queue_processing_ {false};

    SessionId id_ {0};
    std::atomic<bool> closed_ {true};
    std::atomic<bool> closing_ {false};
};


template <typename SessionType> requires std::is_base_of_v<Session, SessionType>
SessionFactory Session::MakeSessionFactory()
{
    return [](boost::asio::io_context& io_context, Socket&& socket) {
        return std::make_shared<SessionType>(io_context, std::move(socket));
    };
}
}
