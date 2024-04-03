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
    constexpr uint32_t PASSWORD = 123456789;
    constexpr MessageType SIMPLE_MESSAGE_TYPE = 0x77;

    boost::asio::io_context io_context {};

    SimpleServer server {
        ServerConfig(PORT),
        io_context,
        [](boost::asio::io_context& io_context, tcp::socket&& socket) {
            return std::make_shared<SimpleClient>(io_context, std::move(socket), 0);
        }
    };

    Connection client_connection {io_context, tcp::socket {io_context}};

    auto message_handler = [&server, &client_connection](std::unique_ptr<Message> message, std::shared_ptr<IConnection>) {
        SKYMARLIN_LOG_INFO("Simple message is about to handled");
        switch (message->header().type) {
            case SIMPLE_MESSAGE_TYPE: {
                auto simple_message = GetSimpleMessage(message->buffer().data());
                if (simple_message->password() != PASSWORD) {
                    FAIL();
                }

                server.Stop();
                client_connection.Disconnect();
                break;
            }
            default:
                FAIL();
            break;
        }
    };
    MessageHandler::InitHandler(std::move(message_handler));

    server.Start();

    co_spawn(io_context, [&client_connection]()-> boost::asio::awaitable<void> {
        if (const auto result = co_await Connection::Connect(client_connection, "localhost", PORT); !result) {
            FAIL();
        }

        // Send SimpleMessage
        flatbuffers::FlatBufferBuilder builder(32);
        builder.Pad(MessageHeader::HEADER_SIZE);

        auto simple_message = CreateSimpleMessage(builder, PASSWORD);
        FinishSimpleMessageBuffer(builder, simple_message);

        auto buffer = std::make_shared<flatbuffers::DetachedBuffer>(builder.Release());
        MessageHeader::WriteHeader(
            std::span<byte, MessageHeader::HEADER_SIZE>(buffer->data(), MessageHeader::HEADER_SIZE), {
                .type = SIMPLE_MESSAGE_TYPE, .size = static_cast<MessageSize>(buffer->size())
            });

        client_connection.SendMessage(std::move(buffer));
        SKYMARLIN_LOG_INFO("Sending simple message...");
    }, boost::asio::detached);

    io_context.run();
}
}
