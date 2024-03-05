#include <skymarlin/net/Client.hpp>

namespace skymarlin::net
{
Client::Client(boost::asio::io_context& io_context, Socket&& socket, const ClientId id)
    : connection_(io_context, std::move(socket)), id_(id) {}

void Client::Start()
{
    running_ = true;
    connection_.StartReceiveMessage();

    OnStart();
}

void Client::Stop()
{
    if (!running_) return;
    running_ = false;

    connection_.Disconnect();

    OnStop();
}
}
