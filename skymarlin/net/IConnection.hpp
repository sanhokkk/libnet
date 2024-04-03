#pragma once

#include <boost/asio.hpp>

namespace skymarlin::net {
using boost::asio::ip::tcp;

class IConnection {
public:
    virtual ~IConnection() = default;

    virtual void Disconnect() = 0;
    virtual void StartReceiveMessage() = 0;
    virtual void SendMessage(std::shared_ptr<flatbuffers::DetachedBuffer> message) = 0;

    virtual bool connected() const = 0;
    virtual tcp::endpoint local_endpoint() const = 0;
    virtual tcp::endpoint remote_endpoint() const = 0;
};
}