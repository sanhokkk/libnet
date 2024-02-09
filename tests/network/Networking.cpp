#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <skymarlin/network/Client.hpp>
#include <skymarlin/network/Listener.hpp>
#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/network/Server.hpp>
#include <skymarlin/network/Session.hpp>
#include <skymarlin/utility/MutableByteBuffer.hpp>

namespace skymarlin::network::test
{
class TestPacket final : public Packet
{
public:
    TestPacket() = default;

    explicit TestPacket(const u32 user_id)
        :user_id_(user_id) {}

    void Serialize(byte* dest) const override
    {
        utility::MutableByteBuffer buffer(dest, length());
        buffer << user_id_;
    };

    void Deserialize(byte* src) override
    {
        const utility::ConstByteBuffer buffer(src, length());
        buffer >> user_id_;
    };

    void Handle(std::shared_ptr<Session> session) override
    {
        SKYMARLIN_LOG_INFO("Handling TestPacket");
    }

    PacketLength length() const override
    {
        return sizeof(decltype(user_id_));
    }

    PacketHeader header() const override
    {
        return {.length = this->length(), .type = Type, .dummy = 0};
    }

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
    void OnClose() override {}
};

class TestServer final : public Server, std::enable_shared_from_this<TestServer>
{
public:
    explicit TestServer(ServerConfig&& config)
        : Server(std::move(config), MakeOnAccpetFunction()) {}

    ~TestServer() override = default;

    void AddSession(const std::shared_ptr<Session>& session)
    {
        sessions_.push_back(session);
    }

    std::mutex stopped_mtx;
    std::condition_variable stopped_cv;
    bool stopped {false};

private:
    Listener::OnAcceptFunction MakeOnAccpetFunction()
    {
        return [this] (tcp::socket&& socket) {
            const auto new_session = std::make_shared<TestSession>(io_context_, std::move(socket));
            new_session->Open();
            AddSession(new_session);

            SKYMARLIN_LOG_INFO("Hello from server...");
            auto hello_packet = std::make_unique<TestPacket>(888);
            new_session->SendPacket(std::move(hello_packet));
        };
    }

    std::vector<std::shared_ptr<Session>> sessions_;
};

class TestClient final : public Client
{
public:
    explicit TestClient(ClientConfig&& config)
        : Client(std::move(config)) {}

    ~TestClient() override = default;

    void OnConnect(tcp::socket&& socket) override
    {
        session_ = std::make_shared<TestSession>(io_context_, std::move(socket));
        session_->Open();
    }

    void OnStop() override
    {
        {
            std::lock_guard lock(stopped_mtx);
            stopped = true;
        }
        stopped_cv.notify_one();
    }

    std::mutex stopped_mtx;
    std::condition_variable stopped_cv;
    bool stopped {false};
};

TEST(Networking, Connection)
{
    constexpr auto port = static_cast<unsigned short>(50000);

    const auto make_packet_factories = [] {
        return std::vector {
            PacketResolver::MakePacketFactory<TestPacket>(TestPacket::Type),
        };
    };
    PacketResolver::Register(make_packet_factories());

    auto make_server_config = [] {
        return ServerConfig {
            .listen_port = port
        };
    };
    auto server = TestServer(make_server_config());
    server.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto client = TestClient({"localhost", port});
    client.Start();

    {
        std::unique_lock lock(client.stopped_mtx);
        client.stopped_cv.wait(lock, [&client] { return client.stopped; });
    }

    {
        std::unique_lock lock(server.stopped_mtx);
        server.stopped_cv.wait(lock, [&server] { return server.stopped; });
    }
}
}
