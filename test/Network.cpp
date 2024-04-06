#include <catch2/catch_test_macros.hpp>
#include <skymarlin/net/Client.hpp>
#include <skymarlin/net/Connection.hpp>
#include <skymarlin/net/Server.hpp>

#include "SimpleMessage.hpp"

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

    void OnStop() override {}

    void HandleMessage(std::vector<uint8_t>&& buffer) override {
        //TODO: verify buffer
        flatbuffers::Verifier verifier {buffer.data(), buffer.size()};
        bool ok = VerifyMessageBuffer(verifier);

        const auto message = GetMessage(buffer.data());
        bool ok2 = VerifyMessageType(verifier, message, MessageType::SimpleMessage);

        const auto message_type = message->message_type();
        if (message_type != MessageType::SimpleMessage) {
            FAIL();
        }

        const auto simple_message = message->message_as<SimpleMessage>();
        SKYMARLIN_LOG_INFO("hello world: {} {}", simple_message->hello()->c_str(), simple_message->world()->c_str());
    }
};

class SimpleServer final : public Server {
public:
    SimpleServer(ServerConfig&& config, boost::asio::io_context& io_context, ClientFactory&& client_factory)
        : Server(std::move(config), io_context, std::move(client_factory)) {}

private:
    void OnStart() override {}
    void OnStop() override {}
};

TEST_CASE("Simple message exchange", "Network") {
    constexpr unsigned short PORT = 55555;

    boost::asio::io_context server_context {};

    SimpleServer server {
        ServerConfig(PORT),
        server_context,
        [](boost::asio::io_context& io_context, tcp::socket&& socket) {
            return std::make_shared<SimpleClient>(io_context, std::move(socket), 0);
        }
    };

    boost::asio::io_context client_context {};
    ConsumerQueue<std::vector<uint8_t>> client_message_queue {
        client_context, [](std::vector<uint8_t>&&)-> boost::asio::awaitable<const boost::system::error_code> {
            co_return boost::system::error_code {};
        },
        [](const boost::system::error_code&) {}
    };
    Connection client_connection {client_context, tcp::socket {client_context}, client_message_queue};

    server.Start();

    co_spawn(client_context, [&client_connection]()-> boost::asio::awaitable<void> {
        if (const auto result = co_await Connection::Connect(client_connection, "localhost", PORT); !result) {
            FAIL();
        }

        // Send SimpleMessage
        flatbuffers::FlatBufferBuilder builder(64);

        auto simple_message = CreateSimpleMessage(builder, builder.CreateString("hello"), builder.CreateString("world"));
        builder.FinishSizePrefixed(CreateMessage(builder, MessageType::SimpleMessage, simple_message.Union()));

        SKYMARLIN_LOG_INFO("Send {} bytes", builder.GetSize());

        client_connection.SendMessage(std::make_shared<flatbuffers::DetachedBuffer>(builder.Release()));
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
