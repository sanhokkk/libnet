#pragma once

#include <skymarlin/network/Listener.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
struct ServerConfig
{
    // TODO: Read from config file
    const unsigned short listen_port;
};


class Server : boost::noncopyable
{
public:
    Server() = delete;
    Server(ServerConfig&& config, Listener::OnAcceptFunction&& on_accept);
    virtual ~Server() = default;

    void Start();
    void Stop();

    bool running() const { return running_; }

protected:
    const ServerConfig config_;
    boost::asio::io_context io_context_ {};
    std::unique_ptr<Listener> listener_;
    std::atomic<bool> running_ {false};
};

inline Server::Server(ServerConfig&& config, Listener::OnAcceptFunction&& on_accept)
    : config_(std::move(config)),
    listener_(std::make_unique<Listener>(io_context_, config_.listen_port, std::move(on_accept))) {}

inline void Server::Start()
{
    running_ = true;

    if (!listener_) {
        SKYMARLIN_LOG_CRITICAL("Listener is not initialized");
        exit(1);
    }
    listener_->Start();

    std::thread([this] {
        while (running_) {
            io_context_.run();
        }
        SKYMARLIN_LOG_INFO("IO thread terminating");
    }).detach();
}

inline void Server::Stop()
{
    SKYMARLIN_LOG_INFO("Stopping the server...");
    running_ = false;

    listener_->Stop();
    io_context_.stop();
}
}
