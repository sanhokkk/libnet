#include <skymarlin/network/Server.hpp>

namespace skymarlin::network
{
Server::Server(const ServerConfig& config)
    : listener_(io_context_, config.listen_port) {}

void Server::Run()
{
    listener_.Run();
    io_context_.run();
}
}
