#pragma once

#include <boost/asio.hpp>
#include <skymarlin/network/Session.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;

class Connector
{
public:
    template <typename SessionType> requires std::is_base_of_v<Session, SessionType>
    static std::future<std::shared_ptr<Session>> Connect(boost::asio::io_context& io_context,
        const tcp::resolver::results_type& endpoints)
    {
        auto socket = tcp::socket(io_context);
        boost::asio::async_connect(socket, endpoints, [](const boost::system::error_code& ec, const tcp::endpoint& remote_endpoint) {

        });

        std::future<std::shared_ptr<Session>> session = std::async(std::launch::async, [&io_context, &endpoints] {
            auto socket = tcp::socket(io_context);
            try {
                socket.connect(endpoints);
            }
            catch (const boost::system::system_error& e) {
                SKYMARLIN_LOG_ERROR("Error connecting to {}:{}; {}", remote_endpoint.address().to_string(), remote_endpoint.port(), e.what());
                return nullptr;
            }

            return std::make_shared<SessionType>(std::move(socket));
        });

        return session;
    }
};
}
