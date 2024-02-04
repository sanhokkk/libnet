#pragma once

#include <skymarlin/network/Listener.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
struct ServerConfig
{
    short listen_port;
    // TODO: Read from config file
};

class Server : boost::noncopyable
{
public:
    Server() = delete;
    explicit Server(ServerConfig config);
    virtual ~Server() = default;

    std::thread Start();
    void Stop();

    bool running() const { return running_; }

protected:
    const ServerConfig config_;
    boost::asio::io_context io_context_ {};
    std::unique_ptr<Listener> listener_;
    std::atomic<bool> running_ {false};

private:
    virtual void Init() = 0;
};

inline Server::Server(const ServerConfig config)
    : config_(config) {}

inline std::thread Server::Start()
{
    Init();
    if (!listener_) {
        SKYMARLIN_LOG_CRITICAL("Listener is not initialized");
        exit(1);
    }

    listener_->Start();
    std::thread io_thread([this] {
        while (running_) {
            io_context_.run();
        }
        SKYMARLIN_LOG_INFO("IO thread terminating");
    });

    running_ = true;
    return io_thread;
}

inline void Server::Stop()
{
    SKYMARLIN_LOG_INFO("Stopping the server...");
    running_ = false;

    listener_->Stop();
    io_context_.stop();
}
}
