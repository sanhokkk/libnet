#pragma once

#include <boost/asio.hpp>
#include <skymarlin/net/Connection.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::net
{
class Connector
{
    static boost::asio::awaitable<std::optional<Connection>> Connect(
        boost::asio::io_context& io_context, Socket&& socket, std::string_view host, uint16_t port)
    {
        auto resolver = tcp::resolver(io_context);

        SKYMARLIN_LOG_INFO("Trying to connect to {}:{}", host, port);
        const auto [ec, endpoints] = co_await resolver.async_resolve(host,
            std::format("{}", port), as_tuple(boost::asio::use_awaitable));
        if (ec) {
            SKYMARLIN_LOG_ERROR("Error on resolve: {}", ec.what());
            co_return std::nullopt;
        }

        if (const auto [ec, endpoint] = co_await async_connect(socket.lowest_layer(), endpoints,
            as_tuple(boost::asio::use_awaitable)); ec) {
            SKYMARLIN_LOG_ERROR("Error on connect: {}", ec.what());
            co_return std::nullopt;
        }

        if (const auto [ec] = co_await socket.async_handshake(boost::asio::ssl::stream_base::client,
            as_tuple(boost::asio::use_awaitable)); ec) {
            SKYMARLIN_LOG_ERROR("Error on handshake: {}", ec.what());
            co_return std::nullopt;
        }

        co_return Connection {io_context, std::move(socket)};
    }
};
}
