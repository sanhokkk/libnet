#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/net/Connection.hpp>

namespace skymarlin::net
{
class Client;

using ClientId = uint32_t;
using ClientFactory = std::function<std::shared_ptr<Client>(boost::asio::io_context&, Socket&&)>;

class Client : boost::noncopyable
{
public:
    Client(boost::asio::io_context& io_context, Socket&& socket, ClientId id);
    virtual ~Client() = default;

    void Start();
    void Stop();

    virtual void OnStart() = 0;
    virtual void OnStop() = 0;

    ClientId id() const { return id_; }
    bool running() const { return running_; }
    tcp::endpoint remote_endpoint() const { return connection_.remote_endpoint(); }

protected:
    Connection connection_;

private:
    const ClientId id_;
    bool running_ {false};
};
}
