#pragma once

#include <future>

#include <skymarlin/network/Listener.hpp>

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
        std::cout << "listener is not initialized" << std::endl;
        exit(1);
    }

    listener_->Start();
    running_ = true;

    std::thread io_thread([this] {
        while (running_) {
            io_context_.run();
        }
        std::cout << "IO thread terminating" << std::endl;
    });

    return io_thread;
}

inline void Server::Stop()
{
    std::cout << "Stopping the server..." << std::endl;
    running_ = false;

    listener_->Stop();
    io_context_.stop();
}
}
