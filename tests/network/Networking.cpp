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

#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <skymarlin/net/ClientManager.hpp>
#include <skymarlin/net/Server.hpp>
#include <skymarlin/utility/Log.hpp>

#include <chrono>
#include <thread>

namespace skymarlin::net::test
{
class TestClient final : public Client
{
public:
    TestClient(boost::asio::io_context& io_context, Socket&& socket)
        : Client(io_context, std::move(socket), ++id_generator_) {}

    ~TestClient() override = default;

    void OnStart() override {};
    void OnStop() override {};

private:
    inline static std::atomic<ClientId> id_generator_ {0};
};

class TestServer final : public Server
{
public:
    explicit TestServer(ServerConfig&& config, boost::asio::io_context& io_context)
        : Server(std::move(config), io_context, [](boost::asio::io_context& io_context, Socket&& socket) {
            return std::make_shared<TestClient>(io_context, std::move(socket));
        }) {}

    ~TestServer() override = default;
};

TEST(Networking, StartAndStopServer)
{
    boost::asio::io_context io_context {};
    auto io_work_guard = make_work_guard(io_context);

    std::thread server_thread([&io_context] {
        auto make_server_config = []() -> ServerConfig {
            return {
                .listen_port = 55555,
                .ssl_certificate_chain_file = std::format("{}/tests/network/server.test.crt", PROJECT_ROOT),
                .ssl_private_key_file = std::format("{}/tests/network/server.test.key", PROJECT_ROOT)
            };
        };

        auto server = TestServer(make_server_config(), io_context);
        server.Start();
        server.Stop();

        io_context.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    io_work_guard.reset();

    if (server_thread.joinable()) server_thread.join();
}
}
