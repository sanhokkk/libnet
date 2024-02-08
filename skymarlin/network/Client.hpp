#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Session.hpp>
#include <skymarlin/utility/Log.hpp>

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
    explicit Client(ClientConfig&& config);
    virtual ~Client() = default;

    void Start();
    void Stop();

    bool running() const { return running_; }

private:
    boost::asio::awaitable<tcp::socket> Connect();
    virtual void OnConnect(tcp::socket&& socket) = 0;
    virtual void OnStop() = 0;

protected:
    const ClientConfig config_;
    boost::asio::io_context io_context_ {};
    std::shared_ptr<Session> session_;

private:
    tcp::endpoint remote_endpoint_;
    std::atomic<bool> running_ {false};
};

inline Client::Client(ClientConfig&& config)
    : config_(std::move(config)) {}

inline void Client::Start()
{
    running_ = true;

    std::thread([this] {
        while (running_) {
            io_context_.run();
        }
        SKYMARLIN_LOG_INFO("IO thread terminating");
    }).detach();


    co_spawn(io_context_,
        [this]() -> boost::asio::awaitable<void> {
            auto socket = co_await Connect();
            OnConnect(std::move(socket));
        },
        boost::asio::detached
    );
}

inline void Client::Stop()
{
    running_ = false;
}

inline boost::asio::awaitable<tcp::socket> Client::Connect()
{
    auto socket = tcp::socket(io_context_);
    auto resolver = tcp::resolver(io_context_);
    try {
        SKYMARLIN_LOG_INFO("Trying resolve {}:{}", config_.remote_adress, config_.remote_port);
        const auto endpoints = co_await resolver.async_resolve(config_.remote_adress,
            std::format("{}", config_.remote_port), boost::asio::use_awaitable);

        co_await async_connect(socket, endpoints, boost::asio::use_awaitable);
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error on connect: {}", e.what());
        Stop();
    }

    co_return socket;
}
}
