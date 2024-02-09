#include <skymarlin/network/Client.hpp>

#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
Client::Client(ClientConfig&& config, SessionFactory&& session_factory)
    : config_(std::move(config)), session_factory_(std::move(session_factory)) {}

void Client::Start()
{
    running_ = true;

    co_spawn(io_context_,
        [this]() -> boost::asio::awaitable<void> {
            auto socket = co_await Connect();
            session_ = session_factory_(io_context_, std::move(socket));
            session_->Open();
        },
        boost::asio::detached
    );

    while (running_) {
        io_context_.run();
    }
}

void Client::Stop()
{
    SKYMARLIN_LOG_INFO("Stopping the client...");
    running_ = false;

    session_->Close();
    io_context_.stop();
}

boost::asio::awaitable<tcp::socket> Client::Connect()
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