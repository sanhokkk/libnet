#include <skymarlin/network/Listener.hpp>

#include <iostream>
#include <skymarlin/network/Session.hpp>

namespace skymarlin::network {
Listener::Listener(boost::asio::io_context& io_context, const short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v6(), port)) {
}

void Listener::Run() {
    std::cout << "Start accepting on: " << acceptor_.local_endpoint() << std::endl;
    StartAccept();
}

void Listener::StartAccept() {
    acceptor_.async_accept([this](const boost::system::error_code& ec, tcp::socket socket) {
        if (ec) {
            std::cout << "Error accepting socket: " << ec.message() << std::endl;
        } else {
            std::cout << socket.remote_endpoint() << " connecting to "
                    << socket.local_endpoint() << '\n';

            // TODO: Register to session manager
            Session::Create(std::move(socket))->Run();
        }

        StartAccept();
    });
}
}
