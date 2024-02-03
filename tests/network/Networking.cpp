#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include <boost/asio.hpp>
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
        : Session(std::move(socket)) {}

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

private:
    void Init() override
    {
        listener_ = std::make_unique<TestListener>(io_context_, config_.listen_port);
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

    void Handle(std::shared_ptr<Session> session) override
    {
    }

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

TEST(Networking, LaunchServer)
{
    auto make_config = [] {
        return network::ServerConfig {
            static_cast<short>(33333),
        };
    };

    auto server = TestServer(make_config());
    auto io_thread = server.Start();
    server.Stop();

    if (io_thread.joinable()) {
        io_thread.join();
    }
}


TEST(Networking, Connection)
{
    /*auto LaunchServer = [] {
        auto make_config = [] {
            return network::ServerConfig {
                static_cast<short>(33333),
            };
        };

        auto server = TestServer(make_config());
        auto io_thread = server.Start();

        if (io_thread.joinable()) {
            io_thread.join();
        }
    };

    auto LaunchClient = [] {
        boost::asio::io_context io_context {};
        Connector connector(io_context);

        connector.Connect();
        // std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if (!connector.connected()) {
            FAIL();
        }
    };

    std::thread server_thread(LaunchServer, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::thread client_thread(LaunchClient, false);

    server_thread.join();
    client_thread.join();*/
}
}
