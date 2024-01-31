#pragma once

#include <skymarlin/network/Listener.hpp>
#include <skymarlin/network/Session.hpp>

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
    explicit Server(ServerConfig config);
    virtual ~Server() = 0;

    virtual void Init() = 0;

    void Run();

private:
    const ServerConfig config_;
    boost::asio::io_context io_context_{};
    std::unique_ptr<Listener> listener_{nullptr};
};

inline Server::Server(const ServerConfig config)
    : config_(config) {}

inline void Server::Run()
{
    listener_->Run();
    io_context_.run();
}
}
