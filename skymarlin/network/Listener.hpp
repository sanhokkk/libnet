#pragma once

#include <iostream>

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;

class Listener : boost::noncopyable
{
public:
    Listener() = delete;

    Listener(boost::asio::io_context& io_context, short port);

    virtual ~Listener();

    void Run();

protected:
    virtual void OnAccept(tcp::socket&& socket) = 0;

private:
    void Accept();

    tcp::acceptor acceptor_;
};

inline Listener::Listener(boost::asio::io_context& io_context, const short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v6(), port)) {}

inline void Listener::Run()
{
    std::cout << "Start accepting on: " << acceptor_.local_endpoint() << std::endl;
    Accept();
}

inline void Listener::Accept()
{
    acceptor_.async_accept([this](const boost::system::error_code& ec, tcp::socket socket) {
        if (ec) {
            std::cout << "Error accepting socket: " << ec.message() << std::endl;
        }
        else {
            //TODO: Use format
            std::cout << socket.remote_endpoint() << " connecting to " << socket.local_endpoint() << std::endl;

            OnAccept(std::move(socket));
        }

        Accept();
    });
}
}
