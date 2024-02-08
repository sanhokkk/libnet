#include <skymarlin/network/Session.hpp>

#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/utility/ByteBufferExceptions.hpp>

namespace skymarlin::network
{
Session::Session(boost::asio::io_context& io_context, tcp::socket&& socket)
    : io_context_(io_context), socket_(std::move(socket))
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

void Session::Open()
{
    closed_ = false;
    closing_ = false;

    const auto receive_coroutine = [](std::shared_ptr<Session> self) -> boost::asio::awaitable<void> {
        while (self->open()) {
            if (auto packet = co_await self->ReceivePacket()) {
                packet->Handle(self->shared_from_this());
            }
        }
        SKYMARLIN_LOG_INFO("Session receive coroutine terminating");
    };
    co_spawn(io_context_, receive_coroutine(shared_from_this()), boost::asio::detached);
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

void Session::SendPacket(std::unique_ptr<Packet> packet)
{
    if (!packet) return;

    const auto send_coroutine = [this](std::unique_ptr<Packet> _packet) -> boost::asio::awaitable<void> {
        //TODO: buffer pooling?
        std::vector<byte> buffer(PACKET_HEADER_SIZE + _packet->length());
        Packet::WriteHeader(buffer.data(), _packet->header());
        _packet->Serialize(buffer.data() + PACKET_HEADER_SIZE);

        const auto [ec, _] = co_await socket_.async_send(
            boost::asio::buffer(buffer), as_tuple(boost::asio::use_awaitable));
        if (ec) {
            SKYMARLIN_LOG_ERROR("Error on send packet: {}", ec.what());
            Close();
            co_return;
        }
    };
    co_spawn(io_context_, send_coroutine(std::move(packet)), boost::asio::detached);
}


boost::asio::awaitable<std::unique_ptr<Packet>> Session::ReceivePacket()
{
    const PacketHeader header = co_await ReadPacketHeader();
    if (!header) {
        Close();
        co_return nullptr;
    }

    std::unique_ptr<Packet> packet = PacketResolver::Resolve(header.type);
    if (!packet) {
        SKYMARLIN_LOG_ERROR("Invalid packet type: {}", header.type);
        Close();
        co_return nullptr;
    }

    boost::asio::mutable_buffer buffer;
    try {
        buffer = receive_streambuf_.prepare(header.length);
    }
    catch (const std::length_error& e) {
        SKYMARLIN_LOG_ERROR("Not enough receive buffer: {}", e.what());
    }

    if (!co_await ReadPacketBody(buffer)) {
        Close();
        co_return nullptr;
    }

    receive_streambuf_.commit(header.length);
    try {
        packet->Deserialize(boost::asio::buffer_cast<byte*>(buffer));
    }
    catch (const std::exception& e) {
        SKYMARLIN_LOG_ERROR("Error on deserializing packet: {}", e.what());
        Close();
        co_return nullptr;
    }
    receive_streambuf_.consume(header.length);

    co_return packet;
}


boost::asio::awaitable<PacketHeader> Session::ReadPacketHeader()
{
    const auto [ec, _] =
        co_await socket_.async_receive(boost::asio::buffer(header_buffer_), as_tuple(boost::asio::use_awaitable));

    if (ec) {
        SKYMARLIN_LOG_ERROR("Error on reading packet header: {}", ec.what());
        co_return PacketHeader {};
    }

    co_return Packet::ReadHeader(header_buffer_);
}

boost::asio::awaitable<bool> Session::ReadPacketBody(boost::asio::mutable_buffer buffer)
{
    const auto [ec, bytes_transferred] = co_await socket_.async_receive(buffer, as_tuple(boost::asio::use_awaitable));

    if (ec) {
        SKYMARLIN_LOG_ERROR("Error on reading packet body: {}", ec.what());
        co_return false;
    }

    co_return true;
}
}
