#pragma once

#include <boost/asio.hpp>
#include <skymarlin/net/client_manager.hpp>
#include <skymarlin/net/listener.hpp>

namespace skymarlin::net {
struct ServerConfig {
    const unsigned short listen_port;
};


class Server {
public:
    Server(ServerConfig&& config, boost::asio::io_context& ctx);
    virtual ~Server() = default;

    void start();
    void stop();

    bool running() const { return running_; }

protected:
    const ServerConfig config_;
    boost::asio::io_context& ctx_;

private:
    virtual void on_start() = 0;
    virtual void on_stop() = 0;

    Listener listener_;
    std::atomic<bool> running_ {false};
};


inline Server::Server(ServerConfig&& config, boost::asio::io_context& ctx)
    : config_(std::move(config)),
      ctx_(ctx),
      listener_(ctx_, config_.listen_port) {}

inline void Server::start() {
    running_ = true;
    listener_.start();

    on_start();
}

inline void Server::stop() {
    running_ = false;

    listener_.stop();
    ClientManager::clients.clear();

    on_stop();
}
}
