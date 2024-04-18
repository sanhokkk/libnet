#pragma once

#include <boost/asio.hpp>
#include <skymarlin/net/ClientManager.hpp>
#include <skymarlin/net/Listener.hpp>

namespace skymarlin::net {
struct ServerConfig {
    const unsigned short listen_port;
};


class Server : boost::noncopyable {
public:
    Server(ServerConfig&& config, boost::asio::io_context& io_context);
    virtual ~Server() = default;

    void Start();
    void Stop();

    bool running() const { return running_; }

protected:
    const ServerConfig config_;
    boost::asio::io_context& io_context_;

private:
    virtual void OnStart() = 0;
    virtual void OnStop() = 0;

    Listener listener_;
    std::atomic<bool> running_ {false};
};


inline Server::Server(ServerConfig&& config, boost::asio::io_context& io_context)
    : config_(std::move(config)),
      io_context_(io_context),
      listener_(io_context_, config_.listen_port) {}

inline void Server::Start() {
    running_ = true;
    listener_.Start();

    OnStart();
}

inline void Server::Stop() {
    running_ = false;

    listener_.Stop();
    ClientManager::ClearClients();

    OnStop();
}
}
