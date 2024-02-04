#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Session.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
class Connector : boost::noncopyable
{
public:
    explicit Connector(boost::asio::io_context& io_context);

    void Connect();
    void Disconnect();

    bool connected() const { return connected_; }

private:
    void OnConnect(const tcp::endpoint& endpoint);
    // void OnDisconnect();

    std::atomic<bool> connected_{false};

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
                SKYMARLIN_LOG_ERROR("Error connecting: {}", ec.what());
                return;
            }

            OnConnect(endpoint);
        });
}

inline void Connector::Disconnect()
{
    try {
        SKYMARLIN_LOG_INFO("Disconnecting from {}:{}", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        socket_.close();
    }
    catch (const boost::system::system_error &e) {
        SKYMARLIN_LOG_ERROR("Error closing socket: {}", e.what());
    }
    connected_ = false;
}


inline void Connector::OnConnect(const tcp::endpoint& endpoint)
{
    connected_ = true;

    SKYMARLIN_LOG_INFO("Connected to {}:{}", endpoint.address().to_string(), endpoint.port());
}
}
