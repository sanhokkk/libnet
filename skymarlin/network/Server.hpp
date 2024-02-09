#pragma once

#include <skymarlin/network/Listener.hpp>

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
    explicit Server(ServerConfig&& config);
    virtual ~Server() = default;

    void Start();
    void Stop();

    bool running() const { return running_; }

protected:
    const ServerConfig config_;
    boost::asio::io_context io_context_ {};
    Listener listener_;
    std::atomic<bool> running_ {false};
};
}
