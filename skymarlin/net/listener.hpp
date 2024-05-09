#pragma once

#include <boost/asio.hpp>
#include <skymarlin/net/client.hpp>
#include <skymarlin/net/client_manager.hpp>
#include <spdlog/spdlog.h>

namespace skymarlin::net {
using boost::asio::ip::tcp;

class Listener final : boost::noncopyable {
public:
    Listener(boost::asio::io_context& ctx, unsigned short port);
    ~Listener() = default;

    void start();
    void stop();

private:
    boost::asio::awaitable<void> listen();

    boost::asio::io_context& ctx_;
    tcp::acceptor acceptor_;

    std::atomic<bool> listening_ {false};
};

inline Listener::Listener(boost::asio::io_context& ctx, const unsigned short port)
    : ctx_(ctx),
    acceptor_(ctx, tcp::endpoint(tcp::v6(), port)) {}

inline void Listener::start() {
    listening_ = true;
    co_spawn(ctx_, listen(), boost::asio::detached);
}

inline void Listener::stop() {
    if (!listening_.exchange(false)) return;

    try {
        acceptor_.close();
    }
    catch (const boost::system::system_error& e) {
        spdlog::error("[Listener] Error closing: {}", e.what());
    }
}

inline boost::asio::awaitable<void> Listener::listen() {
    spdlog::info("[Listener] Start accepting on {}:{}", acceptor_.local_endpoint().address().to_string(),
        acceptor_.local_endpoint().port());

    while (listening_) {
        tcp::socket socket {ctx_};

        if (const auto [ec] = co_await acceptor_.async_accept(socket, as_tuple(boost::asio::use_awaitable)); ec) {
            spdlog::error("[Listener] Error on accepting: {}", ec.what());
            continue;
        }

        const auto _ = ClientManager::create_client(ctx_, std::move(socket));
    }
}
}
