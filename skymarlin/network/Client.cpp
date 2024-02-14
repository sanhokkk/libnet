/*
 * MIT License
 *
 * Copyright (c) 2024 skymarlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <skymarlin/network/Client.hpp>

#include <skymarlin/network/Log.hpp>

namespace skymarlin::network
{
Client::Client(ClientConfig&& config, boost::asio::io_context& io_context, SessionFactory&& session_factory)
    : config_(std::move(config)), io_context_(io_context), session_factory_(std::move(session_factory))
{
    ssl_context_.set_default_verify_paths();
}

void Client::Start()
{
    running_ = true;

    co_spawn(io_context_, Connect(), boost::asio::detached);
    // OnStart();
}

void Client::Stop()
{
    if (!running_.exchange(false)) return;

    SKYMARLIN_LOG_INFO("Stopping the client...");
    running_ = false;

    if (session_) {
        co_spawn(io_context_, session_->Close(), boost::asio::detached);
    }
}

boost::asio::awaitable<void> Client::Connect()
{
    auto socket = Socket(io_context_, ssl_context_);
    auto resolver = tcp::resolver(io_context_);

    SKYMARLIN_LOG_INFO("Trying to connect to {}:{}", config_.remote_adress, config_.remote_port);
    const auto [ec, endpoints] = co_await resolver.async_resolve(config_.remote_adress,
        std::format("{}", config_.remote_port), as_tuple(boost::asio::use_awaitable));
    if (ec) {
        SKYMARLIN_LOG_ERROR("Error on resolve: {}", ec.what());
        Stop();
        co_return;
    }

    if (const auto [ec, endpoint] = co_await async_connect(socket.lowest_layer(), endpoints,
        as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on connect: {}", ec.what());
        Stop();
        co_return;
    }

    if (const auto [ec] = co_await socket.async_handshake(boost::asio::ssl::stream_base::client,
        as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on handshake: {}", ec.what());
        Stop();
        co_return;
    }

    session_ = session_factory_(io_context_, std::move(socket));
    session_->Open();
}
}
