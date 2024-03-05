#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <skymarlin/net/Listener.hpp>

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
}
