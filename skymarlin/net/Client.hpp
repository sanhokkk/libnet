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
    void SendMessage(std::shared_ptr<Message> message);

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
