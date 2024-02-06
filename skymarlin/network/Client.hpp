#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Connector.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;

struct ClientConfig
{
    const std::string remote_adress;
    const short remote_port;
};

class Client : boost::noncopyable
{
public:
    explicit Client(ClientConfig&& config);
    virtual ~Client();

    std::thread Start();
    void Stop();
    virtual void Connect() = 0;

    bool running() const { return running_; }

protected:
    const ClientConfig config_;
    boost::asio::io_context io_context_ {};

private:
    tcp::endpoint remote_endpoint_;
    std::atomic<bool> running_ {false};
};

inline Client::Client(ClientConfig&& config)
    : config_(config)
{
    tcp::resolver::results_type endpoints;
    try {
        tcp::resolver resolver {io_context_};
        endpoints = resolver.resolve(config_.remote_adress, std::format("{}", config_.remote_port));
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_CRITICAL("Cannot resolve remote endpoint: {}", e.what());
        exit(1);
    }


}

inline std::thread Client::Start()
{
    running_ = true;

    std::thread io_thread([this] {
        while (running_) {
            io_context_.run();
        }
        SKYMARLIN_LOG_INFO("IO thread terminating");
    });

    return io_thread;
}

inline void Client::Stop()
{
    running_ = false;
}
}
