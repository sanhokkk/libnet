#pragma once

#include <iostream>
#include <memory>

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Packet.hpp>
#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/thread/Queue.hpp>
#include <skymarlin/utility/ByteBufferExceptions.hpp>
#include <skymarlin/utility/MutableByteBuffer.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>, boost::noncopyable
{
public:
    explicit Session(tcp::socket&& socket);

    virtual ~Session();

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
    byte header_buffer_source_[PACKET_HEADER_SIZE]{};
    utility::ConstByteBuffer header_buffer_;

    thread::ConcurrentQueue<boost::asio::const_buffer> write_queue_;
    std::atomic<bool> writing_{false};

    std::atomic<bool> closed_, closing_;
};

inline Session::Session(tcp::socket&& socket)
    : socket_(std::move(socket)), header_buffer_(header_buffer_source_, PACKET_HEADER_SIZE),
      closed_(false), closing_(false)
{
    boost::system::error_code ec;
    socket_.set_option(tcp::no_delay(true), ec);
    if (ec) {
        std::cout << "Error setting socket no-delay: " << ec.what() << std::endl;
    }
}


inline Session::~Session()
{
    closed_ = true;

    boost::system::error_code ec;
    socket_.close(ec);
    if (ec) {
        std::cout << "Error closing socket: " << ec.what() << std::endl;
    }
}

inline void Session::Run()
{
    ReadHeader();
}

inline void Session::Close()
{
    if (closed_.exchange(true))
        return;

    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_send, ec);
    if (ec) {
        std::cout << "Error shutting down socket: " << ec.what() << std::endl;
    }

    OnClose();
}

inline bool Session::IsOpen() const
{
    return !closed_ && !closing_;
}

inline void Session::ReadHeader()
{
    if (!IsOpen())
        return;

    boost::asio::async_read(socket_,
        boost::asio::buffer(header_buffer_source_),
        boost::asio::transfer_exactly(PACKET_HEADER_SIZE),
        [self = shared_from_this()](const boost::system::error_code& ec,
        const size_t bytes_transferred) {
            if (ec) {
                std::cout << "Error reading packet header: " << ec.message() << std::endl;
                self->Close();
                return;
            }
            if (bytes_transferred == 0) {
                std::cout << "Error reading packet header: zero bytes read for packet header" <<
                    std::endl;
                self->Close();
                return;
            }

            self->OnReadHeader();
        });
}

inline void Session::OnReadHeader()
{
    PacketLength packet_length;
    PacketType packet_type;
    try {
        header_buffer_ >> packet_length >> packet_type;
    }
    catch (const utility::ByteBufferException& e) {
        std::cout << "Error reading bytes on header: " << e.what() << std::endl;
        Close();
        return;
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        Close();
        return;
    }

    std::shared_ptr<boost::asio::mutable_buffer> buffer;
    try {
        buffer = std::make_shared<boost::asio::mutable_buffer>(read_streambuf_.prepare(packet_length));
    }
    catch (const std::length_error& e) {
        std::cout << "Insuffcient buffer size to prepare: " << e.what() << std::endl;
        Close();
        return;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        Close();
        return;
    }

    boost::asio::async_read(socket_,
        *buffer,
        boost::asio::transfer_exactly(packet_length),
        [self = shared_from_this(), packet_length, packet_type, buffer](
        const boost::system::error_code& ec, const size_t bytes_transferred) {
            if (ec) {
                std::cout << "Error reading packet body: " << ec.message() << std::endl;
                self->Close();
                return;
            }
            if (bytes_transferred != packet_length) {
                std::cout <<
                    "Error reading packet body: Inconsistent bytes between read bytes and packet length"
                    <<
                    std::endl;
                self->Close();
                return;
            }

            self->OnReadPacket(packet_type, *buffer);
            self->read_streambuf_.consume(bytes_transferred);
        });

    ReadHeader();
}

inline void Session::OnReadPacket(const PacketType packet_type, const boost::asio::mutable_buffer& buffer)
{
    const std::shared_ptr<Packet> packet = PacketResolver::Resolve(packet_type);
    if (!packet) {
        std::cout << "Invalid packet type: " << packet_type << std::endl;
        Close();
        return;
    }

    packet->Deserialize(buffer);
    packet->Handle(shared_from_this());
}

inline void Session::Write(boost::asio::const_buffer& buffer)
{
    write_queue_.Push(std::move(buffer));

    if (writing_.exchange(true))
        return;

    // TODO: Extract? & Lauch as async
    while (!write_queue_.Empty()) {
        boost::asio::async_write(socket_, write_queue_.Pop(),
            [self = shared_from_this()](const boost::system::error_code& ec,
            const size_t bytes_transferred) {
                if (ec) {
                    std::cout << "Error writing packet header: " << ec.message() << std::endl;
                    self->Close();
                    return;
                }
                if (bytes_transferred == 0) {
                    std::cout <<
                        "Error writing packet header: zero bytes written for packet header" <<
                        std::endl;
                    self->Close();
                    return;
                }
            });
    }
    writing_ = false;
}
}
