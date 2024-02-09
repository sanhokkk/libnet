#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Session.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;


struct ClientConfig
{
    const std::string remote_adress;
    const unsigned short remote_port;
};


class Client : boost::noncopyable
{
public:
    explicit Client(ClientConfig&& config, SessionFactory&& session_factory);
    virtual ~Client() = default;

    void Start();
    void Stop();

    bool running() const { return running_; }

private:
    boost::asio::awaitable<tcp::socket> Connect();

protected:
    const ClientConfig config_;
    boost::asio::io_context io_context_ {};
    std::shared_ptr<Session> session_;

private:
    SessionFactory session_factory_;
    tcp::endpoint remote_endpoint_;
    std::atomic<bool> running_ {false};
};
}
