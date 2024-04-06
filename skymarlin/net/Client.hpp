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

    boost::asio::awaitable<void> ProcessReceiveQueue();

    boost::asio::io_context& io_context_;

    const ClientId id_;
    std::atomic<bool> running_ {false};
    util::ConcurrentQueue<std::vector<uint8_t>> receive_queue_ {};
    std::atomic<bool> receive_queue_processing_ {false};

    Connection connection_;
};

//TODO: Extract consume function and error handling function
inline Client::Client(boost::asio::io_context& io_context, tcp::socket&& socket, const ClientId id)
    : io_context_(io_context), id_(id),
      connection_(io_context, std::move(socket), receive_queue_, [this] {
          if (receive_queue_processing_.exchange(true)) return;
          co_spawn(io_context_, ProcessReceiveQueue(), boost::asio::detached);
      }) {}

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

inline boost::asio::awaitable<void> Client::ProcessReceiveQueue() {
    while (!receive_queue_.empty()) {
        HandleMessage(receive_queue_.Pop());
    }

    receive_queue_processing_ = false;
}
}
