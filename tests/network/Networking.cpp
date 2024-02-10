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

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <skymarlin/network/Client.hpp>
#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/network/Server.hpp>
#include <skymarlin/network/Session.hpp>
#include <skymarlin/utility/Log.hpp>
#include <skymarlin/utility/MutableByteStream.hpp>

namespace skymarlin::network::test
{
class TestPacket final : public Packet
{
public:
    TestPacket() = default;

    explicit TestPacket(const u32 user_id)
        : user_id_(user_id) {}

    bool Serialize(byte* dest, const size_t) const override
    {
        utility::MutableByteStream buffer(dest, length());
        buffer << user_id_;
        return true;
    };

    bool Deserialize(const byte* src, const size_t) override
    {
        const utility::ConstByteStream buffer(const_cast<byte*>(src), length());
        buffer >> user_id_;
        return true;
    };

    void Handle(std::shared_ptr<Session>) override
    {
        SKYMARLIN_LOG_INFO("Handling TestPacket");
        // session->Close();
    }

    PacketLength length() const override { return sizeof(decltype(user_id_)); }

    PacketHeader header() const override { return {.length = this->length(), .type = Type, .crypto = 0}; }

    constexpr static PacketType Type = 77;

private:
    u32 user_id_ {};
};

class TestSession final : public Session
{
public:
    explicit TestSession(boost::asio::io_context& io_context, tcp::socket&& socket)
        : Session(io_context, std::move(socket))
    {
        SKYMARLIN_LOG_INFO("Session created on {}", local_endpoint().address().to_string());
    }

protected:
    void OnOpen() override
    {
        auto hello_packet = std::make_unique<TestPacket>(42);
        SendPacket(std::move(hello_packet));
    }

    void OnClose() override
    {
        // SKYMARLIN_LOG_INFO("Session closed on {}:{}", local_endpoint().address().to_string(), local_endpoint().port());
    }
};

class TestServer final : public Server, std::enable_shared_from_this<TestServer>
{
public:
    explicit TestServer(ServerConfig&& config)
        : Server(std::move(config), Session::MakeSessionFactory<TestSession>()) {}

    ~TestServer() override = default;
};

class TestClient final : public Client
{
public:
    explicit TestClient(ClientConfig&& config)
        : Client(std::move(config), Session::MakeSessionFactory<TestSession>()) {}

    // ~TestClient() override = default;
};

TEST(Networking, StartAndStopServer)
{
    auto server = TestServer({33333});
    std::thread server_thread([&server] {
        server.Start();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    server.Stop();

    server_thread.join();
}

TEST(Networking, StartAndStopClient)
{
    auto client = TestClient({"localhost", 55555}); // should faild to connect
    client.Start();
}

TEST(Networking, Connection)
{
    constexpr auto port = static_cast<unsigned short>(50000);

    const auto make_packet_factories = [] {
        return std::vector {
            PacketResolver::MakePacketFactory<TestPacket>(TestPacket::Type),
        };
    };
    PacketResolver::Register(make_packet_factories());

    auto server = TestServer({port});
    std::thread server_thread([&server] {
        server.Start();
    });

    auto client = TestClient({"localhost", port});
    std::thread client_thread([&client] {
        client.Start();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client.Stop();
    client_thread.join();

    server.Stop();
    server_thread.join();
}
}
