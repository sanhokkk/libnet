#include <skymarlin/network/Listener.hpp>

#include <iostream>
#include <skymarlin/network/Session.hpp>

namespace skymarlin::network {
Listener::Listener(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v6(), port)) {
}

void Listener::Run() {
    std::cout << "Start accepting on: " << acceptor_.local_endpoint() << std::endl;
    start_accept();
}

void Listener::start_accept() {
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::cout << socket.remote_endpoint() << " connecting to "
                    << socket.local_endpoint() << '\n';

            std::make_shared<Session>(std::move(socket))->run();
        } else {
            std::cout << "error: " << ec.message() << std::endl;
        }

        start_accept();
    });
}
}
