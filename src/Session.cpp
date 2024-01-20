#include <skymarlin/network/Session.hpp>

#include <iostream>
#include <utility>

#include <skymarlin/network/packet/ByteBufferExceptions.hpp>
#include <skymarlin/network/packet/PacketResolver.hpp>

namespace skymarlin::network {
Session::Session(tcp::socket&& socket)
    : socket_(std::move(socket)), header_buffer_(header_buffer_source_, packet::PACKET_HEADER_SIZE),
      closed_(false), closing_(false) {
    boost::system::error_code ec;
    socket_.set_option(tcp::no_delay(true), ec);
    if (ec) {
        std::cout << "Error setting socket no-delay: " << ec.what() << std::endl;
    }
}

Session::~Session() {
    closed_ = true;

    boost::system::error_code ec;
    socket_.close(ec);
    if (ec) {
        std::cout << "Error closing socket: " << ec.what() << std::endl;
    }
}

void Session::Run() {
    AsyncReadHeader();
}

void Session::Close() {
    if (closed_.exchange(true))
        return;

    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_send, ec);
    if (ec) {
        std::cout << "Error shutting down socket: " << ec.what() << std::endl;
    }

    OnClose();
}

bool Session::IsOpen() const {
    return !closed_ && !closing_;
}

void Session::AsyncReadHeader() {
    if (!IsOpen())
        return;

    boost::asio::async_read(socket_,
        boost::asio::buffer(header_buffer_source_),
        boost::asio::transfer_exactly(packet::PACKET_HEADER_SIZE),
        [self = shared_from_this()](const boost::system::error_code& ec, const size_t bytes_transferred) {
            if (ec) {
                std::cout << "Error reading packet header: " << ec.message() << std::endl;
                self->Close();
                return;
            }
            if (bytes_transferred <= 0) {
                std::cout << "Error reading packet header: zero bytes read for packet header" << std::endl;
                self->Close();
                return;
            }

            self->OnReadHeader();
        });
}

void Session::OnReadHeader() {
    PacketLength packet_length;
    PacketType packet_type;
    try {
        header_buffer_ >> packet_length >> packet_type;
    } catch (const packet::ByteBufferException& e) {
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
        buffer = std::make_shared<boost::asio::mutable_buffer>(buffer_.prepare(packet_length));
    } catch (const std::length_error& e) {
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
                std::cout << "Error reading packet body: Inconsistent bytes between read bytes and packet length" <<
                        std::endl;
                self->Close();
                return;
            }

            self->OnReadPacket(packet_length, packet_type, *buffer);
        });

    AsyncReadHeader();
}

void Session::OnReadPacket(PacketLength packet_length, PacketType packet_type,
    const boost::asio::mutable_buffer& buffer) {
    // auto packet = std::make_shared<packet::Packet>(packet_length, packet_type); //TODO: Resolve packet, build from factory

    packet::PacketResolver::PacketCreator packet_creator;
    packet::PacketResolver::PacketHandler packet_handler;
    if (!packet::PacketResolver::TryGetPacketCreator(packet_type, packet_creator)
        || !packet::PacketResolver::TryGetPacketHandler(packet_type, packet_handler)) {
        std::cout << "Invalid packet type: " << packet_type << std::endl;
        return;
    }

    auto packet = packet_creator(packet_length, packet_type);
    packet->Deserialize(packet::ConstByteBuffer(static_cast<byte*>(buffer.data()), buffer.size()));
    packet_handler(std::move(packet)); // TODO: async call
}
}
