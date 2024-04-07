#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <skymarlin/net/Client.hpp>
#include <skymarlin/net/Connection.hpp>
#include <skymarlin/net/Server.hpp>

#include "SimpleMessage.hpp"

#include <chrono>

namespace skymarlin::net::test {
class SimpleClient final : public Client {
public:
    SimpleClient(boost::asio::io_context& io_context, tcp::socket&& socket, const ClientId id)
        : Client(io_context, std::move(socket), id) {}

private:
    void OnStart() override {
        SKYMARLIN_LOG_INFO("SimpleClient OnStart connected to {}:{}", remote_endpoint().address().to_string(),
                           remote_endpoint().port());
    }

    void OnStop() override {
        SKYMARLIN_LOG_INFO("SimpleClient stopping");
    }

    void HandleMessage(std::vector<uint8_t>&& buffer) override {
        flatbuffers::Verifier verifier {buffer.data(), buffer.size()};
        CHECK(VerifyMessageBuffer(verifier));

        const auto message = GetMessage(buffer.data());
        const auto message_type = message->message_type();
        CHECK(message_type == MessageType::SimpleMessage);

        const auto simple_message = message->message_as<SimpleMessage>();
        SKYMARLIN_LOG_INFO("hello world: {} {}", simple_message->hello()->c_str(), simple_message->world()->c_str());

        Stop();
    }
};

class SimpleServer final : public Server {
public:
    SimpleServer(ServerConfig&& config, boost::asio::io_context& io_context, ClientFactory&& client_factory)
        : Server(std::move(config), io_context, std::move(client_factory)) {}

private:
    void OnStart() override {
        co_spawn(io_context_, [this]()->boost::asio::awaitable<void> {
            boost::asio::steady_timer timer(io_context_, std::chrono::milliseconds(500));
            co_await timer.async_wait(boost::asio::use_awaitable);

            Stop();
        }, boost::asio::detached);
    }
    void OnStop() override {
        SKYMARLIN_LOG_INFO("SimpleServer stopping");
    }
};

TEST_CASE("Simple message exchange") {
    constexpr unsigned short PORT = 55555;

    boost::asio::io_context server_context {};
    SimpleServer server {
        ServerConfig(PORT),
        server_context,
        [](boost::asio::io_context& io_context, tcp::socket&& socket) {
            return std::make_shared<SimpleClient>(io_context, std::move(socket), 0);
        }
    };
    server.Start();

    boost::asio::io_context client_context {};
    util::ConcurrentQueue<std::vector<uint8_t>> client_receive_queue {}; // Not used
    Connection client_connection {client_context, tcp::socket {client_context}, client_receive_queue, [] {}};

    co_spawn(client_context, [&client_connection]()-> boost::asio::awaitable<void> {
        {
            const auto result = co_await Connection::Connect(client_connection, "localhost", PORT);
            CHECK(result);
        }

        // Send SimpleMessage
        flatbuffers::FlatBufferBuilder builder(64);

        auto simple_message =
                CreateSimpleMessage(builder, builder.CreateString("hello"), builder.CreateString("world"));
        builder.FinishSizePrefixed(CreateMessage(builder, MessageType::SimpleMessage, simple_message.Union()));

        auto buffer = std::make_shared<flatbuffers::DetachedBuffer>(builder.Release());
        SKYMARLIN_LOG_INFO("Send {} bytes", buffer->size());
        client_connection.SendMessage(buffer);
    }, boost::asio::detached);

    std::thread t1([&server_context] {
        server_context.run();
    });
    std::thread t2([&client_context] {
        client_context.run();
    });

    t1.join();
    t2.join();
}
}
