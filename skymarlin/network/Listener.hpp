#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;

class Listener : boost::noncopyable
{
public:
    Listener(boost::asio::io_context& io_context, short port);
    virtual ~Listener() = default;

    void Start();
    void Stop();

private:
    void Accept();
    virtual void OnAccept(tcp::socket&& socket) = 0;

    tcp::acceptor acceptor_;
};

inline Listener::Listener(boost::asio::io_context& io_context, const short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v6(), port)) {}


inline void Listener::Start()
{
    SKYMARLIN_LOG_INFO("Start accepting on {}:{}", acceptor_.local_endpoint().address().to_string(), acceptor_.local_endpoint().port());

    Accept();
}

inline void Listener::Stop()
{
    try {
        acceptor_.close();
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing listener: {}", e.what());
    }
}


inline void Listener::Accept()
{
    acceptor_.async_accept([this](const boost::system::error_code& ec, tcp::socket socket) {
        if (ec) {
            SKYMARLIN_LOG_ERROR("Error accepting socket: {}", ec.message());
        }
        else {
            SKYMARLIN_LOG_INFO("Accepted from {}:{}", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());

            OnAccept(std::move(socket));
        }

        Accept();
    });
}
}
