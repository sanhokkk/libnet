#include <skymarlin/network/Server.hpp>

#include <skymarlin/network/SessionManager.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
Server::Server(ServerConfig&& config, SessionFactory&& session_factory)
    : config_(std::move(config)), listener_(io_context_, config.listen_port, std::move(session_factory)) {}

void Server::Start()
{
    running_ = true;
    listener_.Start();

    while (running_) {
        io_context_.run();
    }
}

void Server::Stop()
{
    SKYMARLIN_LOG_INFO("Stopping the server...");
    running_ = false;

    listener_.Stop();
    //TODO: Close all sessions

    io_context_.stop();
}
}
