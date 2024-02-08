#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Packet.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>, boost::noncopyable
{
public:
    explicit Session(tcp::socket&& socket);
    virtual ~Session();

    void Open(boost::asio::io_context& io_context);
    void Close();
    void SendPacket(std::unique_ptr<Packet> packet);

    bool open() const { return !closed_ && !closing_; };
    tcp::endpoint local_endpoint() const { return socket_.local_endpoint(); }
    tcp::endpoint remote_endpoint() const { return socket_.remote_endpoint(); }

protected:
    virtual void OnClose() = 0;

private:
    boost::asio::awaitable<std::unique_ptr<Packet>> ReceivePacket();
    boost::asio::awaitable<PacketHeader> ReadPacketHeader();
    boost::asio::awaitable<boost::asio::mutable_buffer> ReadPacketBody(PacketLength length);

    tcp::socket socket_;
    boost::asio::streambuf receive_streambuf_;
    byte header_buffer_[PACKET_HEADER_SIZE] {};

    std::atomic<bool> closed_ {true};
    std::atomic<bool> closing_ {false};
};
}
