#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <skymarlin/network/Client.hpp>
#include <skymarlin/network/Listener.hpp>
#include <skymarlin/network/Server.hpp>
#include <skymarlin/network/Session.hpp>
#include <skymarlin/utility/MutableByteBuffer.hpp>

namespace skymarlin::network::test
{
class TestServer;
class TestListener;

class TestSession final : public Session
{
public:
    explicit TestSession(tcp::socket&& socket)
        : Session(std::move(socket))
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
            const auto new_session = std::make_shared<TestSession>(std::move(socket));
            AddSession(new_session);
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

    void OnConnect(tcp::socket&& socket) override {}

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

class TestPacket final : public Packet
{
public:
    enum class Action : byte
    {
        GREET,
        CLOSE
    };

    /*TestPacket(const u32 user_id, const Action action, const f64 dummy)
        :user_id_(user_id), action_(action), dummy_(dummy) {}*/

    void Serialize(byte* dest) const override
    {
        // bytebuffer << user_id_ << action_ << dummy_;
    };

    void Deserialize(const byte* src) override
    {
        // bytebuffer >> user_id_ >> action_ >> dummy_;
    };

    void Handle(std::shared_ptr<Session> session) override {}

    size_t length() const override
    {
        return sizeof(decltype(user_id_)) + sizeof(decltype(action_)) + sizeof(decltype(dummy_));
    };

    static constexpr PacketType Type = 0x11;

private:
    u32 user_id_ {};
    Action action_ {};
    f64 dummy_ {};
};

TEST(Networking, Connection)
{
    constexpr auto port = static_cast<unsigned short>(50000);

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
