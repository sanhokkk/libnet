#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <skymarlin/net/ClientManager.hpp>
#include <skymarlin/net/Listener.hpp>
#include <skymarlin/util/Log.hpp>

namespace skymarlin::net
{
struct ServerConfig
{
    // TODO: Read from config file
    const unsigned short listen_port;
    const std::string ssl_certificate_chain_file;
    const std::string ssl_private_key_file;
};


class Server : boost::noncopyable
{
public:
    Server(ServerConfig&& config, boost::asio::io_context& io_context, ClientFactory&& client_factory);
    virtual ~Server() = default;

    void Start();
    void Stop();

    bool running() const { return running_; }

protected:
    const ServerConfig config_;
    boost::asio::io_context& io_context_;
    boost::asio::ssl::context ssl_context_ {boost::asio::ssl::context::tlsv13_server};

private:
    Listener listener_;
    std::atomic<bool> running_ {false};
};


inline Server::Server(ServerConfig&& config, boost::asio::io_context& io_context, ClientFactory&& client_factory)
    : config_(std::move(config)),
    io_context_(io_context),
    listener_(io_context_, ssl_context_, config_.listen_port, std::move(client_factory))
{
    ssl_context_.use_certificate_chain_file(config_.ssl_certificate_chain_file);
    ssl_context_.use_private_key_file(config_.ssl_private_key_file, boost::asio::ssl::context::pem);
}

inline void Server::Start()
{
    running_ = true;
    listener_.Start();
}

inline void Server::Stop()
{
    SKYMARLIN_LOG_INFO("Stopping the server...");
    running_ = false;

    listener_.Stop();
    ClientManager::ClearClients();
}
}
