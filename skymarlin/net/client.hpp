#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/net/connection.hpp>

namespace skymarlin::net {
using ClientId = uint32_t;

class Client : boost::noncopyable {
public:
    Client(boost::asio::io_context& ctx, tcp::socket&& socket, ClientId id);
    virtual ~Client() = default;

    void start();
    void stop();
    void send_message(std::shared_ptr<flatbuffers::DetachedBuffer> message);

    ClientId id() const { return id_; }
    bool running() const { return running_; }
    tcp::endpoint remote_endpoint() const { return connection_.remote_endpoint(); }

protected:
    void set_id(const ClientId id) { id_ = id; }

private:
    virtual void on_start() = 0;
    virtual void on_stop() = 0;
    virtual void handle_message(std::vector<uint8_t>&& buffer) = 0;

    boost::asio::awaitable<void> process_receive_queue();

    boost::asio::io_context& ctx_;

    ClientId id_;
    std::atomic<bool> running_ {false};
    ConcurrentQueue<std::vector<uint8_t>> receive_queue_ {};
    std::atomic<bool> receive_queue_processing_ {false};

    Connection connection_;
};


inline Client::Client(boost::asio::io_context& ctx, tcp::socket&& socket, const ClientId id)
    : ctx_(ctx), id_(id),
    connection_(ctx, std::move(socket), receive_queue_, [this] {
        if (receive_queue_processing_.exchange(true)) return;
        co_spawn(ctx_, process_receive_queue(), boost::asio::detached);
    }) {}

inline void Client::start() {
    running_ = true;

    on_start();
}

inline void Client::stop() {
    if (!running_.exchange(false)) return;

    connection_.disconnect();

    on_stop();
}

inline void Client::send_message(std::shared_ptr<flatbuffers::DetachedBuffer> message) {
    connection_.send_message(std::move(message));
}

inline boost::asio::awaitable<void> Client::process_receive_queue() {
    while (!receive_queue_.empty()) {
        handle_message(receive_queue_.pop());
    }

    receive_queue_processing_ = false;
    co_return;
}
}
