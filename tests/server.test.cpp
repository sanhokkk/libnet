#include <catch2/catch_test_macros.hpp>
#include <skymarlin/net/listener_tcp.hpp>
#include <skymarlin/net/peer_tcp.hpp>
#include <skymarlin/net/peer_udp.hpp>
#include <tests/hello.hpp>

#include <chrono>
#include <cstring>
#include <iostream>

using namespace skymarlin::net;
using namespace skymarlin::net::tests;
using namespace std::chrono_literals;

TEST_CASE("[tcp server]") {
    constexpr unsigned short PORT {50000};
    constexpr std::string_view MESSAGE = "Hello";

    boost::asio::io_context ctx {};
    std::vector<std::unique_ptr<PeerTCP>> clients;

    bool is_message_handled {false};
    const auto message_handler = [&MESSAGE, &is_message_handled](std::vector<uint8_t>&& message) {
        flatbuffers::Verifier verifier {message.data(), message.size()};
        REQUIRE(VerifyHelloBuffer(verifier));

        const auto hello = GetHello(message.data());
        REQUIRE(hello->hello()->size() == MESSAGE.size());
        REQUIRE(memcmp(hello->hello()->data(), MESSAGE.data(), MESSAGE.size()) == 0);
        is_message_handled = true;
    };

    ListenerTCP listener {
        ctx, PORT,
        [&clients, &message_handler](boost::asio::io_context& ctx, tcp::socket&& socket) {
            auto new_client = std::make_unique<PeerTCP>(ctx, std::move(socket), message_handler);
            new_client->run();
            clients.push_back(std::move(new_client));
        }
    };

    SECTION("ListenerTCP accepts a client") {
        listener.start();

        co_spawn(ctx, [&ctx]()->boost::asio::awaitable<void> {
            tcp::socket socket {ctx};
            const auto [ec] = co_await socket.async_connect(
                tcp::endpoint(tcp::v4(), PORT),
                as_tuple(boost::asio::use_awaitable));
            REQUIRE(!ec);
        }, boost::asio::detached);

        ctx.run_for(100ms);
        listener.stop();

        REQUIRE(clients.size() == 1);
        clients.clear();
    }

    SECTION("PeerTCP receives/sends Hello") {
        listener.start();

        co_spawn(ctx, [&ctx]()->boost::asio::awaitable<void> {
            const auto message_handler = [](std::vector<uint8_t>&& message) {};
            PeerTCP client {ctx, tcp::socket {ctx}, message_handler};

            co_await client.connect(tcp::endpoint(tcp::v4(), PORT));
            REQUIRE(client.is_connected());
            client.run();

            flatbuffers::FlatBufferBuilder builder {64};
            builder.FinishSizePrefixed(CreateHello(builder, builder.CreateString(MESSAGE)));
            client.send_message(std::make_shared<flatbuffers::DetachedBuffer>(builder.Release()));
        }, boost::asio::detached);

        ctx.run_for(100ms);
        listener.stop();
        clients.clear();

        REQUIRE(is_message_handled);
    }
}

TEST_CASE("PeerUDP receives/sends Hello", "[udp server]") {
    const udp::endpoint SERVER_ENDPOINT {udp::v4(), 50000};
    const udp::endpoint CLIENT_ENDPOINT {udp::v4(), 50001};
    constexpr std::string_view MESSAGE = "Hello";
    constexpr int PACKETS = 10;

    boost::asio::io_context ctx {};

    int packet_loss = PACKETS;
    const auto packet_handler = [&MESSAGE, &packet_loss](std::vector<uint8_t>&& message) {
        flatbuffers::Verifier verifier {message.data(), message.size()};
        REQUIRE(VerifySizePrefixedHelloBuffer(verifier));

        const auto hello = GetSizePrefixedHello(message.data());
        REQUIRE(hello->hello()->size() == MESSAGE.size());
        REQUIRE(memcmp(hello->hello()->data(), MESSAGE.data(), MESSAGE.size()) == 0);
        packet_loss -= 1;
    };

    PeerUDP server {ctx, SERVER_ENDPOINT, CLIENT_ENDPOINT, packet_handler};
    server.open();

    co_spawn(ctx, [&ctx, &SERVER_ENDPOINT, &CLIENT_ENDPOINT, &MESSAGE, &packet_handler]()->boost::asio::awaitable<void> {
        PeerUDP client {ctx, CLIENT_ENDPOINT, SERVER_ENDPOINT, packet_handler};
        client.open();

        for (int i = 0; i < 10; ++i) {
            flatbuffers::FlatBufferBuilder builder {64};
            builder.FinishSizePrefixed(CreateHello(builder, builder.CreateString(MESSAGE)));

            client.send_packet(std::make_shared<flatbuffers::DetachedBuffer>(builder.Release()));
        }

        co_return;
    }, boost::asio::detached);

    ctx.run_for(100ms);

    REQUIRE(packet_loss < PACKETS);
    std::cout << std::format("{} packets are lost out of {}", packet_loss, PACKETS) << std::endl;
}
