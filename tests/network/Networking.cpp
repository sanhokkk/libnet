#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <skymarlin/network/Client.hpp>
#include <skymarlin/network/Connector.hpp>
#include <skymarlin/network/Listener.hpp>
#include <skymarlin/network/Server.hpp>
#include <skymarlin/network/Session.hpp>
#include <skymarlin/utility/MutableByteBuffer.hpp>

namespace skymarlin::network::test
{
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

class TestListener final : public Listener
{
public:
    TestListener(boost::asio::io_context& io_context, const short listen_port)
        : Listener(io_context, listen_port) {}

    ~TestListener() override = default;

protected:
    void OnAccept(tcp::socket&& socket) override
    {
        auto session = std::make_shared<TestSession>(std::move(socket));
    }
};

class TestServer final : public Server
{
public:
    explicit TestServer(ServerConfig config)
        : Server(config) {}

    ~TestServer() override = default;

    void Init() override
    {
        listener_ = std::make_unique<TestListener>(io_context_, config_.listen_port);
    }
};

class TestClient final : public Client
{
public:
    explicit TestClient(ClientConfig&& config)
        : Client(std::move(config)) {}

    void Connect() override
    {
        Connector::Connect<TestSession>(io_context_, config_.remote_endpoint);
    }
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

    void Serialize(boost::asio::mutable_buffer& buffer) const override
    {
        auto bytebuffer = utility::MutableByteBuffer(buffer);
        // bytebuffer << user_id_ << action_ << dummy_;
    };

    void Deserialize(const boost::asio::mutable_buffer& buffer) override
    {
        const auto bytebuffer = utility::ConstByteBuffer(boost::asio::buffer_cast<byte*>(buffer), buffer.size());
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
    constexpr short port = 33333;

    auto make_server_config = [] {
        return ServerConfig {
            port,
        };
    };
    auto server = TestServer(make_server_config());
    server.Init();
    auto server_worker = server.Start();

    auto make_client_config = [] {
        ;
    };

    boost::asio::io_context client_io_context {};
    auto client_connector = Connector(client_io_context);
    client_connector.Connect();
    while (!client_connector.connected()) {} // Wait unitll client is connected

    server.Stop();
    client_connector.Disconnect();

    if (server_worker.joinable()) {
        server_worker.join();
    }
}
}
