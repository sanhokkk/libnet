#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <skymarlin/network/Connector.hpp>
#include <skymarlin/network/Server.hpp>
#include <skymarlin/network/Listener.hpp>
#include <skymarlin/network/Session.hpp>

namespace skymarlin::network::test
{
class TestServer final : public Server
{

};

class TestListener final : public Listener
{

};

class TestSession final : public Session
{

};


TEST(Connection, Basic)
{
    boost::asio::io_context io_context{};
    Connector connector(io_context);
}
}
