#include <skymarlin/network/Listener.hpp>

#include <iostream>
#include <skymarlin/network/Session.hpp>

namespace skymarlin::network {
Listener::Listener(boost::asio::io_context& io_context, const short port, Session::SessionCreator&& session_creator)
    : acceptor_(io_context, tcp::endpoint(tcp::v6(), port)),
      session_creator_(std::move(session_creator)) {
}

void Listener::Run() {
    std::cout << "Start accepting on: " << acceptor_.local_endpoint() << std::endl;
    StartAccept();
}

void Listener::StartAccept() {
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec) {
            std::cout << "Error accepting socket: " << ec.message() << std::endl;
        } else {
            std::cout << socket.remote_endpoint() << " connecting to "
                    << socket.local_endpoint() << '\n';

            // TODO: Register to session manager
            session_creator_(std::move(socket))->Run();
        }

        StartAccept();
    });
}
}
