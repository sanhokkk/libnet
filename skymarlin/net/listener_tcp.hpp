#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <spdlog/spdlog.h>

namespace skymarlin::net {
using boost::asio::ip::tcp;

class ListenerTCP final : boost::noncopyable {
public:
    ListenerTCP(boost::asio::io_context& ctx, unsigned short port,
        std::function<void(boost::asio::io_context&, tcp::socket&&)>&& client_factory);
    ~ListenerTCP() = default;

    void start();
    void stop();

private:
    boost::asio::awaitable<void> listen();

    boost::asio::io_context& ctx_;
    tcp::acceptor acceptor_;
    std::function<void(boost::asio::io_context&, tcp::socket&&)> client_factory_;

    std::atomic<bool> listening_ {false};
};

inline ListenerTCP::ListenerTCP(boost::asio::io_context& ctx, const unsigned short port,
    std::function<void(boost::asio::io_context&, tcp::socket&&)>&& client_factory)
    : ctx_(ctx), acceptor_(ctx, tcp::endpoint(tcp::v4(), port)),
    client_factory_(std::move(client_factory)) {}

inline void ListenerTCP::start() {
    listening_ = true;
    co_spawn(ctx_, listen(), boost::asio::detached);
}

inline void ListenerTCP::stop() {
    if (!listening_.exchange(false)) return;

    try {
        acceptor_.close();
    } catch (const boost::system::system_error& e) {
        spdlog::error("[ListenerTCP] Error closing: {}", e.what());
    }
}

inline boost::asio::awaitable<void> ListenerTCP::listen() {
    spdlog::info("[ListenerTCP] Starts accepting on {}:{}", acceptor_.local_endpoint().address().to_string(),
        acceptor_.local_endpoint().port());

    while (listening_) {
        tcp::socket socket {ctx_};

        if (const auto [ec] = co_await acceptor_.async_accept(socket, as_tuple(boost::asio::use_awaitable)); ec) {
            spdlog::error("[ListenerTCP] Error on accepting: {}", ec.what());
            continue;
        }

        spdlog::info("[ListenerTCP] Accepts from {}:{}", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());
        client_factory_(ctx_, std::move(socket));
    }
}
}
