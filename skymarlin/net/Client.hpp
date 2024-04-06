#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/net/Connection.hpp>

namespace skymarlin::net {
class Client;

using ClientId = uint32_t;
using ClientFactory = std::function<std::shared_ptr<Client>(boost::asio::io_context&, tcp::socket&&)>;

class Client : boost::noncopyable {
public:
    Client(boost::asio::io_context& io_context, tcp::socket&& socket, ClientId id);

    virtual ~Client() = default;

    void Start();
    void Stop();

    ClientId id() const { return id_; }
    bool running() const { return running_; }
    tcp::endpoint remote_endpoint() const { return connection_.remote_endpoint(); }

private:
    virtual void OnStart() = 0;
    virtual void OnStop() = 0;
    virtual void HandleMessage(std::vector<uint8_t>&& message) = 0;

    const ClientId id_;
    std::atomic<bool> running_ {false};
    ConsumerQueue<std::vector<uint8_t>> message_receive_queue_;
    Connection connection_;
};

//TODO: Extract consume function and error handling function
inline Client::Client(boost::asio::io_context& io_context, tcp::socket&& socket, const ClientId id)
    : id_(id), message_receive_queue_(
          io_context,
          [this](std::vector<uint8_t>&& message)-> boost::asio::awaitable<const boost::system::error_code> {
              HandleMessage(std::move(message));
              co_return boost::system::error_code {};
          }, [this](const boost::system::error_code&) {}),
      connection_(io_context, std::move(socket), message_receive_queue_) {}

inline void Client::Start() {
    running_ = true;
    connection_.StartReceiveMessage();

    OnStart();
}

inline void Client::Stop() {
    if (!running_.exchange(false)) return;

    connection_.Disconnect();

    OnStop();
}
}
