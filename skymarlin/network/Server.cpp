#include <skymarlin/network/Server.hpp>

#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
Server::Server(ServerConfig&& config)
    : config_(std::move(config)), listener_(io_context_, config.listen_port) {}

void Server::Start()
{
    running_ = true;
    listener_.Start();

    std::thread([this] {
        while (running_) {
            io_context_.run();
        }
        SKYMARLIN_LOG_INFO("IO thread terminating");
    }).detach();
}

void Server::Stop()
{
    SKYMARLIN_LOG_INFO("Stopping the server...");
    running_ = false;

    listener_.Stop();
    io_context_.stop();
}
}
