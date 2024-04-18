#include <catch2/catch_test_macros.hpp>
#include <skymarlin/net/Client.hpp>
#include <skymarlin/net/Connection.hpp>
#include <skymarlin/net/Server.hpp>
#include <test/SimpleMessage.hpp>

#include <chrono>

namespace skymarlin::net::test {
class SimpleClient final : public Client {
public:
    SimpleClient(boost::asio::io_context& io_context, tcp::socket&& socket, const ClientId id)
        : Client(io_context, std::move(socket), id) {}

private:
    void OnStart() override {
        spdlog::info("SimpleClient OnStart connected to {}:{}", remote_endpoint().address().to_string(),
                           remote_endpoint().port());
    }

    void OnStop() override {
        spdlog::info("SimpleClient stopping");
    }

    void HandleMessage(std::vector<uint8_t>&& buffer) override {
        flatbuffers::Verifier verifier {buffer.data(), buffer.size()};
        CHECK(VerifyMessageBuffer(verifier));

        const auto message = GetMessage(buffer.data());
        const auto message_type = message->message_type();
        CHECK(message_type == MessageType::SimpleMessage);

        const auto simple_message = message->message_as<SimpleMessage>();
        spdlog::info("hello world: {} {}", simple_message->hello()->c_str(), simple_message->world()->c_str());

        Stop();
    }
};

class SimpleServer final : public Server {
public:
    SimpleServer(ServerConfig&& config, boost::asio::io_context& io_context)
        : Server(std::move(config), io_context) {}

private:
    void OnStart() override {
        co_spawn(io_context_, [this]()-> boost::asio::awaitable<void> {
            boost::asio::steady_timer timer(io_context_, std::chrono::milliseconds(500));
            co_await timer.async_wait(boost::asio::use_awaitable);

            Stop();
        }, boost::asio::detached);
    }

    void OnStop() override {
        spdlog::info("SimpleServer stopping");
    }
};

TEST_CASE("Simple message exchange", "[net]") {
    constexpr unsigned short PORT = 55555;

    boost::asio::io_context server_context {};
    SimpleServer server {
        ServerConfig(PORT),
        server_context,
    };
    ClientManager::Init([](boost::asio::io_context& io_context, tcp::socket&& socket, ClientId id) {
        return std::make_shared<SimpleClient>(io_context, std::move(socket), id);
    });
    server.Start();

    boost::asio::io_context client_context {};
    co_spawn(client_context, [&client_context]()-> boost::asio::awaitable<void> {
        tcp::socket socket {client_context};
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("::1"), PORT));
        SimpleClient client {client_context, std::move(socket), 0};

        // Send SimpleMessage
        flatbuffers::FlatBufferBuilder builder(64);

        auto simple_message =
                CreateSimpleMessage(builder, builder.CreateString("hello"), builder.CreateString("world"));
        builder.FinishSizePrefixed(CreateMessage(builder, MessageType::SimpleMessage, simple_message.Union()));

        auto buffer = std::make_shared<flatbuffers::DetachedBuffer>(builder.Release());
        spdlog::info("Send {} bytes", buffer->size());
        client.SendMessage(buffer);

        co_return;
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
