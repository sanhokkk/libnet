#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Session.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;


class Listener final : boost::noncopyable
{
public:
    Listener(boost::asio::io_context& io_context, unsigned short port);
    ~Listener() = default;

    void Start();
    void Stop();

private:
    boost::asio::awaitable<void> Listen();

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    SessionFactory session_factory_;

    std::atomic<bool> listening_ {false};
};
}
