#include <skymarlin/net/Server.hpp>

#include <skymarlin/net/ClientManager.hpp>
#include <skymarlin/util/Log.hpp>

namespace skymarlin::net
{
Server::Server(ServerConfig&& config, boost::asio::io_context& io_context, ClientFactory&& client_factory)
    : config_(std::move(config)),
    io_context_(io_context),
    listener_(io_context_, ssl_context_, config_.listen_port, std::move(client_factory))
{
    ssl_context_.use_certificate_chain_file(config_.ssl_certificate_chain_file);
    ssl_context_.use_private_key_file(config_.ssl_private_key_file, boost::asio::ssl::context::pem);
}

void Server::Start()
{
    running_ = true;
    listener_.Start();
}

void Server::Stop()
{
    SKYMARLIN_LOG_INFO("Stopping the server...");
    running_ = false;

    listener_.Stop();
    ClientManager::ClearClients();
}
}
