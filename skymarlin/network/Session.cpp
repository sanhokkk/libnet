#include <skymarlin/network/Session.hpp>

#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/utility/ByteBufferExceptions.hpp>

namespace skymarlin::network
{
Session::Session(tcp::socket&& socket)
    : socket_(std::move(socket))
{
    try {
        socket_.set_option(tcp::no_delay(true));
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error setting socket no-delay: {}", e.what());
    }
}


Session::~Session()
{
    Close();

    try {
        socket_.close();
        socket_.shutdown(tcp::socket::shutdown_send);
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing socket: {}", e.what());
    }
}

void Session::Open(boost::asio::io_context& io_context)
{
    closed_ = false;
    closing_ = false;

    const auto receive_coroutine = [this] -> boost::asio::awaitable<void> {
        while (open()) {
            if (std::unique_ptr<Packet> packet = co_await ReceivePacket()) {
                packet->Handle(shared_from_this());
            }
        }
        SKYMARLIN_LOG_INFO("Session receive coroutine terminating");
    };

    co_spawn(io_context, receive_coroutine(), boost::asio::detached);

    //TODO: send_coroutine
}

void Session::Close()
{
    if (closed_.exchange(true)) return;

    try {
        socket_.shutdown(tcp::socket::shutdown_receive);
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing socket: {}", e.what());
    }

    OnClose();
}

boost::asio::awaitable<std::unique_ptr<Packet>> Session::ReceivePacket()
{
    std::unique_ptr<Packet> packet;
    try {
        const auto [length, type, dummy] = co_await ReadPacketHeader();
        const auto buffer = co_await ReadPacketBody(length);

        packet = PacketResolver::Resolve(type);
        if (!packet) {
            SKYMARLIN_LOG_ERROR("Failed to resolve packet type");
            Close();
            co_return nullptr;
        }

        packet->Deserialize(boost::asio::buffer_cast<const byte*>(buffer));

        receive_streambuf_.consume(length);
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet: {}", e.what());
        Close();
        co_return nullptr;
    }
    catch (const utility::ByteBufferException& e) {
        SKYMARLIN_LOG_ERROR("Error on deserializing packet: {}", e.what());
        Close();
        co_return nullptr;
    }
    catch (const std::exception& e) {
        SKYMARLIN_LOG_ERROR("Exception in ReceivePacket: {}", e.what());
    }
    co_return packet;
}


boost::asio::awaitable<PacketHeader> Session::ReadPacketHeader()
{
    co_await socket_.async_receive(boost::asio::buffer(header_buffer_), boost::asio::use_awaitable);
    co_return Packet::ReadHeader(header_buffer_);
}

boost::asio::awaitable<boost::asio::mutable_buffer> Session::ReadPacketBody(const PacketLength length)
{
    auto buffer = receive_streambuf_.prepare(length);
    co_await socket_.async_receive(buffer, boost::asio::use_awaitable);
    receive_streambuf_.commit(length);

    co_return buffer;
}
}
