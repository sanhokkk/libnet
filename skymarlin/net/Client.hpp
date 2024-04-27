#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/net/Connection.hpp>

namespace skymarlin::net {
class Client;

using ClientId = uint32_t;

class Client : boost::noncopyable, protected std::enable_shared_from_this<Client> {
public:
    Client(boost::asio::io_context& ctx, tcp::socket&& socket, ClientId id);
    virtual ~Client() = default;

    void Start();
    void Stop();
    void SendMessage(std::shared_ptr<flatbuffers::DetachedBuffer> message);

    ClientId id() const { return id_; }
    bool running() const { return running_; }
    tcp::endpoint remote_endpoint() const { return connection_.remote_endpoint(); }

protected:
    void set_id(const ClientId id) { id_ = id; }

private:
    virtual void OnStart() = 0;
    virtual void OnStop() = 0;
    virtual void HandleMessage(std::vector<uint8_t>&& buffer) = 0;

    boost::asio::awaitable<void> ProcessReceiveQueue();

    boost::asio::io_context& ctx_;

    ClientId id_;
    std::atomic<bool> running_ {false};
    util::ConcurrentQueue<std::vector<uint8_t>> receive_queue_ {};
    std::atomic<bool> receive_queue_processing_ {false};

    Connection connection_;
};


inline Client::Client(boost::asio::io_context& ctx, tcp::socket&& socket, const ClientId id)
    : ctx_(ctx), id_(id),
    connection_(ctx, std::move(socket), receive_queue_) {}

inline void Client::Start() {
    running_ = true;
    connection_.Start();

    std::thread message_handle_thread([this] {
        while (running_ && connection_.connected()) {
            if (receive_queue_.empty()) continue;
            HandleMessage(receive_queue_.Pop()); //TODO: Channel
        }
        Stop();
    });
    message_handle_thread.detach();

    OnStart();
}

inline void Client::Stop() {
    if (!running_.exchange(false)) return;

    connection_.Disconnect();

    OnStop();
}

inline void Client::SendMessage(std::shared_ptr<flatbuffers::DetachedBuffer> message) {
    connection_.SendMessage(std::move(message));
}
}
