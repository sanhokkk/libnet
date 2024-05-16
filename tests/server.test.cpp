#include <catch2/catch_test_macros.hpp>
#include <skymarlin/net/listener_tcp.hpp>
#include <skymarlin/net/peer_tcp.hpp>
#include <tests/hello.hpp>

#include <chrono>
#include <cstring>

using namespace skymarlin::net;
using namespace skymarlin::net::tests;
using namespace std::chrono_literals;

TEST_CASE("[tcp server]") {
    constexpr unsigned short PORT {33333};
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
            clients.push_back(std::make_unique<PeerTCP>(ctx, std::move(socket), message_handler));
        }
    };

    SECTION("ListenerTCP accepts a client") {
        listener.start();

        co_spawn(ctx, [&ctx]()->boost::asio::awaitable<void> {
            tcp::socket socket {ctx};
            const auto [ec] = co_await socket.async_connect(
                tcp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), PORT),
                as_tuple(boost::asio::use_awaitable));
            REQUIRE(!ec);
        }, boost::asio::detached);

        ctx.run_for(100ms);
        listener.stop();

        REQUIRE(clients.size() == 1);
        clients.clear();
    }

    SECTION("PeerTCP transports Hello") {
        listener.start();

        co_spawn(ctx, [&ctx]()->boost::asio::awaitable<void> {
            tcp::socket socket {ctx};
            const auto [ec1] = co_await socket.async_connect(
                tcp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), PORT),
                as_tuple(boost::asio::use_awaitable));
            REQUIRE(!ec1);

            flatbuffers::FlatBufferBuilder builder {64};
            auto hello = CreateHello(builder, builder.CreateString(MESSAGE));
            builder.FinishSizePrefixed(hello);

            const auto [ec2, _] = co_await socket.async_send(
                boost::asio::buffer(builder.GetBufferPointer(), builder.GetSize()),
                as_tuple(boost::asio::use_awaitable));
            REQUIRE(!ec2);
        }, boost::asio::detached);

        ctx.run_for(100ms);
        listener.stop();
        clients.clear();

        REQUIRE(is_message_handled);
    }
}

TEST_CASE("[udp server]") {
    //
}
