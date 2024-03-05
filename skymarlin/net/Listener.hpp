#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <skymarlin/net/Client.hpp>

namespace skymarlin::net
{
using boost::asio::ip::tcp;


class Listener final : boost::noncopyable
{
public:
    Listener(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context,
        unsigned short port, ClientFactory&& client_factory);
    ~Listener() = default;

    void Start();
    void Stop();

private:
    boost::asio::awaitable<void> Listen();

    boost::asio::io_context& io_context_;
    boost::asio::ssl::context& ssl_context_;
    tcp::acceptor acceptor_;
    const ClientFactory client_factory_;

    std::atomic<bool> listening_ {false};
};
}
