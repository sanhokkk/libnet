#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/packet/Packet.hpp>
#include <skymarlin/network/packet/MutableByteBuffer.hpp>

namespace skymarlin::network {
using boost::asio::ip::tcp;
using packet::PacketLength;
using packet::PacketType;

class Session : public std::enable_shared_from_this<Session>, boost::noncopyable {
public:
    using SessionCreator = std::function<std::shared_ptr<Session>(tcp::socket&&)>;

    explicit Session(tcp::socket&& socket);

    virtual ~Session();

    void Run();

    void Close();

    bool IsOpen() const;

protected:
    virtual void OnClose() = 0;

private:
    void AsyncReadHeader();

    void OnReadHeader();

    void OnReadPacket(PacketLength packet_length, PacketType packet_type, const boost::asio::mutable_buffer& buffer);

    void AsyncSend();

    tcp::socket socket_;
    packet::ConstByteBuffer header_buffer_;
    byte header_buffer_source_[packet::PACKET_HEADER_SIZE]{};
    boost::asio::streambuf buffer_;

    std::atomic<bool> closed_, closing_;
};
}
