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

#include <skymarlin/net/Listener.hpp>

#include <skymarlin/net/Log.hpp>
#include <skymarlin/net/SessionManager.hpp>

namespace skymarlin::net
{
Listener::Listener(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context,
    const unsigned short port, SessionFactory&& session_factory)
    : io_context_(io_context),
    ssl_context_(ssl_context),
    acceptor_(io_context, tcp::endpoint(tcp::v6(), port)),
    session_factory_(std::move(session_factory)) {}

void Listener::Start()
{
    listening_ = true;
    co_spawn(io_context_, Listen(), boost::asio::detached);
}

void Listener::Stop()
{
    if (!listening_.exchange(false)) return;

    try {
        acceptor_.close();
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing listener: {}", e.what());
    }
}

boost::asio::awaitable<void> Listener::Listen()
{
    SKYMARLIN_LOG_INFO("Start listening on {}:{}", acceptor_.local_endpoint().address().to_string(),
        acceptor_.local_endpoint().port());

    while (listening_) {
        Socket socket(io_context_, ssl_context_);

        if (const auto [ec] = co_await acceptor_.async_accept(socket.lowest_layer(),
            as_tuple(boost::asio::use_awaitable)); ec) {
            SKYMARLIN_LOG_ERROR("Error on accepting: {}", ec.what());
            continue;
        }

        if (const auto [ec] = co_await socket.async_handshake(boost::asio::ssl::stream_base::server,
            as_tuple(boost::asio::use_awaitable)); ec) {
            SKYMARLIN_LOG_ERROR("Error on handshaking: {}", ec.what());
            continue;
        }

        auto session = session_factory_(io_context_, std::move(socket));
        if (!session) {
            SKYMARLIN_LOG_ERROR("Failed to create session after accepting");
            co_return;
        }

        session->Open();
        SessionManager::AddSession(session);
    }
}
}
