#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>

namespace skymarlin::network {
using boost::asio::ip::tcp;

class Listener : boost::noncopyable {
public:
    Listener() = delete;

    Listener(boost::asio::io_context& io_context, short port);

    ~Listener() = default;

    void Run();

private:
    void StartAccept();

    tcp::acceptor acceptor_;
};
}
