#pragma once

#include <iostream>

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Session.hpp>

namespace skymarlin::network
{
class Connector : boost::noncopyable
{
public:
    explicit Connector(boost::asio::io_context& io_context);

    void Connect();

    void OnConnect(const tcp::endpoint& endpoint);

private:
    tcp::socket socket_;
    tcp::resolver resolver_;
    tcp::endpoint peer_endpoint_;
};

inline Connector::Connector(boost::asio::io_context& io_context)
    : socket_(io_context), resolver_(io_context) {}

inline void Connector::Connect()
{
    boost::asio::async_connect(
        socket_,
        resolver_.resolve("::", "33333"),
        [this](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
            if (ec) {
                std::cout << "Error connect: " << ec.what() << std::endl;
                return;
            }

            OnConnect(endpoint);
        });
}

inline void Connector::OnConnect(const tcp::endpoint& endpoint)
{
    std::cout << "Connected to " << endpoint.address() << std::endl;
}
}
