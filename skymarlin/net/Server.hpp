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

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <skymarlin/net/Listener.hpp>

namespace skymarlin::net
{
struct ServerConfig
{
    // TODO: Read from config file
    const unsigned short listen_port;
    const std::string ssl_certificate_chain_file;
    const std::string ssl_private_key_file;
};


class Server : boost::noncopyable
{
public:
    Server(ServerConfig&& config, boost::asio::io_context& io_context, ClientFactory&& client_factory);
    virtual ~Server() = default;

    void Start();
    void Stop();

    bool running() const { return running_; }

protected:
    const ServerConfig config_;
    boost::asio::io_context& io_context_;
    boost::asio::ssl::context ssl_context_ {boost::asio::ssl::context::tlsv13_server};

private:
    Listener listener_;
    std::atomic<bool> running_ {false};
};
}
