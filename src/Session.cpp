#include "Session.hpp"

#include <iostream>

#include <skymarlin/network/ByteBufferExceptions.hpp>

namespace skymarlin::network {
Session::Session(tcp::socket socket)
    : socket_(std::move(socket)), header_buffer_(header_buffer_source_, packet::PACKET_HEADER_SIZE) {
}

Session::~Session() {
    if (socket_.is_open()) {
        socket_.shutdown(tcp::socket::shutdown_both);
        socket_.close();
    }
}

void Session::Run() {
    ReadHeader();
}

void Session::ReadHeader() {
    boost::asio::async_read(socket_,
        boost::asio::buffer(header_buffer_source_),
        boost::asio::transfer_exactly(packet::PACKET_HEADER_SIZE),
        [self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
            self->ReadHeaderHandler(ec, bytes_transferred);
        });
}

void Session::ReadHeaderHandler(const boost::system::error_code& ec, size_t /*bytes_transferred*/) {
    if (ec) {
        std::cout << "Error reading packet header: " << ec.message() << std::endl;
        return;
    }

    auto packet = std::make_unique<packet::Packet>(header_buffer_);
    auto buffer = buffer_.prepare(packet->length());

    boost::asio::async_read(socket_,
        buffer,
        boost::asio::transfer_exactly(packet::PACKET_HEADER_SIZE),
        [self = shared_from_this(), &packet, &buffer](
    const boost::system::error_code& _ec, size_t _bytes_transferred) {
            self->ReadPacketHandler(_ec, _bytes_transferred, std::move(packet), buffer);
        });

    ReadHeader();
}

void Session::ReadPacketHandler(const boost::system::error_code& ec, size_t /*bytes_transferred*/,
    std::unique_ptr<packet::Packet> packet, const boost::asio::mutable_buffer buffer) {
    if (ec) {
        std::cout << "Error reading packet body: " << ec.message() << std::endl;
        return;
    }

    const packet::ConstByteBuffer _buffer(buffer);
    try {
        packet->Deserialize(_buffer);
        //TODO: resolve & deserialize packet, handle it
    } catch (const packet::ByteBufferException& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
}
