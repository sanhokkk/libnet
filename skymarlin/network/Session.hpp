#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/packet/Packet.hpp>
#include <skymarlin/network/utility/MutableByteBuffer.hpp>
#include <skymarlin/thread/Queue.hpp>

namespace skymarlin::network {
using boost::asio::ip::tcp;
using packet::PacketLength;
using packet::PacketType;

class Session : public std::enable_shared_from_this<Session>, boost::noncopyable {
public:
    using SessionCreator = std::function<std::shared_ptr<Session>(tcp::socket&&)>;

    explicit Session(tcp::socket&& socket);

    virtual ~Session();

    inline static SessionCreator Create {};

    void Run();

    void Close();

    bool IsOpen() const;

    void Write(boost::asio::const_buffer& buffer);

protected:
    virtual void OnClose() = 0;

private:
    void ReadHeader();

    void OnReadHeader();

    void OnReadPacket(PacketType packet_type, const boost::asio::mutable_buffer& buffer);

    void WriteInternal();

    tcp::socket socket_;
    boost::asio::streambuf read_streambuf_;
    boost::asio::streambuf write_streambuf_;
    byte header_buffer_source_[packet::PACKET_HEADER_SIZE]{};
    utility::ConstByteBuffer header_buffer_;

    thread::ConcurrentQueue<boost::asio::const_buffer> write_queue_;
    std::atomic<bool> writing_{false};

    std::atomic<bool> closed_, closing_;
};
}
