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
    SKYMARLIN_LOG_INFO("Session destructor");
    Close();
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

    OnOpen();
}

void Session::Close()
{
    if (closed_.exchange(true)) return;

    try {
        socket_.shutdown(tcp::socket::shutdown_both);
        socket_.close();
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing socket: {}", e.what());
    }

    OnClose();
}

void Session::SendPacket(std::unique_ptr<Packet> packet)
{
    if (!packet) return;

    const auto send_coroutine = [](std::shared_ptr<Session> self,
        std::unique_ptr<Packet> _packet) -> boost::asio::awaitable<void> {
        //TODO: buffer pooling?
        std::vector<byte> buffer(PACKET_HEADER_SIZE + _packet->length());
        Packet::WriteHeader(buffer.data(), _packet->header());
        _packet->Serialize(buffer.data() + PACKET_HEADER_SIZE);

        const auto [ec, _] = co_await self->socket_.async_send(
            boost::asio::buffer(buffer), as_tuple(boost::asio::use_awaitable));
        if (ec) {
            SKYMARLIN_LOG_ERROR("Error on sending packet: {}", ec.what());
            self->Close();
            co_return;
        }
    };
    co_spawn(io_context_, send_coroutine(shared_from_this(), std::move(packet)), boost::asio::detached);
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

    packet = co_await ReadPacketBody(std::move(packet), header.length);
    if (!packet) {
        Close();
        co_return nullptr;
    }

    co_return packet;
}

boost::asio::awaitable<PacketHeader> Session::ReadPacketHeader()
{
    const auto [ec, _] =
        co_await socket_.async_receive(boost::asio::buffer(header_buffer_), as_tuple(boost::asio::use_awaitable));

    if (ec) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet header: {}", ec.what());
        co_return PacketHeader {};
    }

    co_return Packet::ReadHeader(header_buffer_);
}

boost::asio::awaitable<std::unique_ptr<Packet>> Session::ReadPacketBody(
    std::unique_ptr<Packet> packet, const PacketLength length)
{
    //TODO: buffer pooling?
    std::vector<byte> buffer(length);

    const auto [ec, _] =
        co_await socket_.async_receive(boost::asio::buffer(buffer), as_tuple(boost::asio::use_awaitable));

    if (ec) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet body: {}", ec.what());
        co_return nullptr;
    }

    try {
        packet->Deserialize(buffer.data());
    }
    catch (const std::exception& e) {
        SKYMARLIN_LOG_ERROR("Error on deserializing packet: {}", e.what());
        co_return nullptr;
    }

    co_return packet;
}
}
