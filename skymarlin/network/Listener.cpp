#include <skymarlin/network/Listener.hpp>

#include <skymarlin/network/SessionManager.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
Listener::Listener(boost::asio::io_context& io_context, const unsigned short port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v6(), port)) {}

void Listener::Start()
{
    listening_ = true;
    co_spawn(io_context_, Listen(), boost::asio::detached);
}

void Listener::Stop()
{
    if (!listening_.exchange(false)) return;

    try {
        acceptor_.close();
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing listener: {}", e.what());
    }
}

boost::asio::awaitable<void> Listener::Listen()
{
    SKYMARLIN_LOG_INFO("Start listening on {}:{}", acceptor_.local_endpoint().address().to_string(),
        acceptor_.local_endpoint().port());

    while (listening_) {
        auto [ec, socket] = co_await acceptor_.async_accept(as_tuple(boost::asio::use_awaitable));

        if (ec) {
            SKYMARLIN_LOG_ERROR("Error on accepting: {}", ec.what());
            continue;
        }

        auto session = session_factory_(io_context_, std::move(socket));
        session->Open();
        SessionManager::AddSession(session);
    }
}
}
