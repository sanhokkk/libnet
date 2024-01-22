#include <skymarlin/network/Server.hpp>

namespace skymarlin::network {
Server::Server(ServerConfig&& config)
    : listener_(io_context_, config.listen_port){
    Session::Create = config.session_creator;
}

void Server::Run() {
    listener_.Run();
    io_context_.run();
}
}
