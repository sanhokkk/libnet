#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/packet/Packet.hpp>
#include <skymarlin/network/packet/MutableByteBuffer.hpp>

namespace skymarlin::network
{
    using boost::asio::ip::tcp;

    class Session : public std::enable_shared_from_this<Session>, boost::noncopyable
    {
    public:
        explicit Session(tcp::socket socket);

        ~Session();

        void Run();

    private:
        void ReadHeader();

        void ReadHeaderHandler(const boost::system::error_code &ec, size_t /*bytes_transferred*/);

        void ReadPacketHandler(const boost::system::error_code &ec, size_t /*bytes_transferred*/,
                               std::unique_ptr<packet::Packet> packet, boost::asio::mutable_buffer buffer);

        tcp::socket socket_;
        packet::ConstByteBuffer header_buffer_;
        byte header_buffer_source_[packet::PACKET_HEADER_SIZE]{};
        boost::asio::streambuf buffer_;
    };
}
