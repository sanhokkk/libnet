#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;

class Listener final : boost::noncopyable
{
public:
    using OnAcceptFunction = std::function<void(tcp::socket&& socket)>;

    Listener(boost::asio::io_context& io_context, short port, const OnAcceptFunction& on_accept);
    ~Listener() = default;

    void Start();
    void Stop();

private:
    boost::asio::awaitable<void> Listen();
    OnAcceptFunction OnAccept;

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    std::atomic<bool> listening_ {false};
};

inline Listener::Listener(boost::asio::io_context& io_context, const short port,
    const OnAcceptFunction& on_accept)
    : OnAccept(on_accept), io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v6(), port)) {}


inline void Listener::Start()
{
    SKYMARLIN_LOG_INFO("Start accepting on {}:{}", acceptor_.local_endpoint().address().to_string(),
        acceptor_.local_endpoint().port());

    listening_ = true;
    co_spawn(io_context_, Listen(), boost::asio::detached);
}

inline void Listener::Stop()
{
    if (!listening_.exchange(false)) return;

    try {
        acceptor_.close();
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing listener: {}", e.what());
    }
}

inline boost::asio::awaitable<void> Listener::Listen()
{
    while (listening_) {
        try {
            tcp::socket socket = co_await acceptor_.async_accept(boost::asio::use_awaitable);
            OnAccept(std::move(socket));
        }
        catch (const boost::system::system_error& e) {
            SKYMARLIN_LOG_ERROR("Error on accept: {}", e.what());
        }
        catch (const std::exception& e) {
            SKYMARLIN_LOG_ERROR("Exception in Listener: {}", e.what());
        }
    }
}
}
