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

#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
Client::Client(ClientConfig&& config, boost::asio::io_context& io_context, SessionFactory&& session_factory)
    : config_(std::move(config)), io_context_(io_context), session_factory_(std::move(session_factory)) {}

void Client::Start()
{
    running_ = true;

    co_spawn(io_context_,
        [this]() -> boost::asio::awaitable<void> {
            auto socket = co_await Connect();
            if (!socket) co_return;

            session_ = session_factory_(io_context_, std::move(*socket));
            session_->Open();

            OnStart();
        },
        boost::asio::detached
    );
}

void Client::Stop()
{
    if (!running_.exchange(false)) return;

    SKYMARLIN_LOG_INFO("Stopping the client...");
    running_ = false;

    if (session_) session_->Close();
}

boost::asio::awaitable<std::optional<tcp::socket>> Client::Connect()
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
        co_return std::nullopt;
    }

    co_return socket;
}
}