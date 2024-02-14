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

#include <skymarlin/network/Session.hpp>

#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/network/SessionManager.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
Session::Session(boost::asio::io_context& io_context, Socket&& socket)
    : io_context_(io_context), socket_(std::move(socket))
{
    try {
        socket_.lowest_layer().set_option(tcp::no_delay(true));
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error setting socket no-delay: {}", e.what());
    }
}

Session::~Session()
{
    if (!closed_) {
        SKYMARLIN_LOG_CRITICAL("Session destructing without being closed; Potential memory leak!");
    }
}

void Session::Open()
{
    closed_ = false;
    closing_ = false;

    const auto receive_coroutine = [](std::shared_ptr<Session> self) -> boost::asio::awaitable<void> {
        while (self->open()) {
            if (const auto packet = co_await self->ReceivePacket(); packet) {
                packet->Handle(self->shared_from_this());
            }
        }
    };
    co_spawn(io_context_, receive_coroutine(shared_from_this()), boost::asio::detached);

    OnOpen();
}

boost::asio::awaitable<void> Session::Close()
{
    if (closed_.exchange(true)) co_return;

    if (auto [ec] = co_await socket_.async_shutdown(as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on shutdown socket: {}", ec.what());
    }

    SessionManager::RemoveSession(shared_from_this());

    OnClose();
}

void Session::SendPacket(std::shared_ptr<Packet> packet)
{
    if (!packet) return;

    send_queue_.Push(std::move(packet));

    if (send_queue_processing_.exchange(true)) return;
    co_spawn(io_context_, SendPacketQueue(), boost::asio::detached);
}

boost::asio::awaitable<void> Session::SendPacketQueue()
{
    while (!send_queue_.empty()) {
        const auto packet = send_queue_.Pop();

        //TODO: buffer pooling?
        std::vector<byte> buffer(PACKET_HEADER_SIZE + packet->length());
        PacketHeader header = packet->header();

        Packet::WriteHeader(buffer.data(), header);
        if (!packet->Serialize(buffer.data() + PACKET_HEADER_SIZE, header.length)) {
            SKYMARLIN_LOG_ERROR("Failed to serialize packet body");
            co_await Close();
            co_return;
        }

        if (const auto [ec, _] = co_await async_write(socket_,
            boost::asio::buffer(buffer), as_tuple(boost::asio::use_awaitable)); ec) {
            SKYMARLIN_LOG_ERROR("Error on sending packet: {}", ec.what());
            co_await Close();
            co_return;
        }
    }

    send_queue_processing_ = false;
}

void Session::BroadcastPacket(std::shared_ptr<Packet> packet)
{
    SessionManager::ForEachSession([packet = std::move(packet)](const std::shared_ptr<Session>& session) {
        session->SendPacket(packet);
    });
}

boost::asio::awaitable<std::shared_ptr<Packet>> Session::ReceivePacket()
{
    const PacketHeader header = co_await ReadPacketHeader();
    if (!header) {
        co_await Close();
        co_return nullptr;
    }

    std::shared_ptr<Packet> packet = PacketResolver::Resolve(header.protocol);
    if (!packet) {
        SKYMARLIN_LOG_ERROR("Invalid packet protocol: {}", header.protocol);
        co_await Close();
        co_return nullptr;
    }

    packet = co_await ReadPacketBody(std::move(packet), header.length);
    if (!packet) {
        co_await Close();
        co_return nullptr;
    }

    co_return packet;
}

boost::asio::awaitable<PacketHeader> Session::ReadPacketHeader()
{
    if (const auto [ec, _] = co_await async_read(socket_,
        boost::asio::buffer(header_buffer_), as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet header: {}", ec.what());
        co_return PacketHeader {};
    }

    co_return Packet::ReadHeader(header_buffer_);
}

boost::asio::awaitable<std::shared_ptr<Packet>> Session::ReadPacketBody(
    std::shared_ptr<Packet> packet, const PacketLength length)
{
    //TODO: buffer pooling?
    std::vector<byte> buffer(length);

    if (const auto [ec, _] = co_await async_read(socket_,
        boost::asio::buffer(buffer), as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet body: {}", ec.what());
        co_return nullptr;
    }

    if (!packet->Deserialize(buffer.data(), length)) {
        SKYMARLIN_LOG_ERROR("Failed to deserialize packet body");
        co_return nullptr;
    }

    co_return packet;
}
}
