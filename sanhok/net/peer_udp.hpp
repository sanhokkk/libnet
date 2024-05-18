#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <flatbuffers/detached_buffer.h>
#include <sanhok/bip_buffer.hpp>
#include <sanhok/concurrent_queue.hpp>
#include <spdlog/spdlog.h>

namespace sanhok::net {
using boost::asio::ip::udp;

class PeerUDP : boost::noncopyable {
public:
    PeerUDP(boost::asio::io_context& ctx, const udp::endpoint& local_endpoint, const udp::endpoint& remote_endpoint,
        std::function<void(std::vector<uint8_t>&&)>&& packet_handler, size_t receive_buffer_size);
    ~PeerUDP();

    void open();
    void close();
    void send_packet(std::shared_ptr<flatbuffers::DetachedBuffer> packet);

    bool is_open() const { return is_open_; }
    udp::endpoint remote_endpoint() const { return socket_.remote_endpoint(); }

private:
    boost::asio::awaitable<void> receive_packet();

    boost::asio::io_context& ctx_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::atomic<bool> is_open_ {false};

    const size_t receive_buffer_size_;
    ConcurrentQueue<std::vector<uint8_t>> receive_queue_;
    std::function<void(std::vector<uint8_t>&&)> packet_handler_;

    std::thread worker_;
};

inline PeerUDP::PeerUDP(boost::asio::io_context& ctx,
    const udp::endpoint& local_endpoint, const udp::endpoint& remote_endpoint,
    std::function<void(std::vector<uint8_t>&&)>&& packet_handler, const size_t receive_buffer_size = 65536)
    : ctx_(ctx), socket_(ctx_, local_endpoint),
    remote_endpoint_(remote_endpoint),
    receive_buffer_size_(receive_buffer_size), packet_handler_(std::move(packet_handler)) {}

inline PeerUDP::~PeerUDP() {
    close();
    if (worker_.joinable()) worker_.join();
}

inline void PeerUDP::open() {
    if (is_open_) return;

    // Start receiving packets
    co_spawn(ctx_, [this]()->boost::asio::awaitable<void> {
        while (is_open_) {
            co_await receive_packet();
        }
    }, boost::asio::detached);

    // Start handling packets
    worker_ = std::thread([this] {
        while (is_open_) {
            auto packet = receive_queue_.pop_wait();
            if (!packet) return;
            packet_handler_(std::move(*packet));
        }
    });
    worker_.detach();

    is_open_ = true;
}

inline void PeerUDP::close() {
    if (!is_open_.exchange(false)) return;

    receive_queue_.clear();

    try {
        socket_.close();
    } catch (const boost::system::system_error& e) {
        spdlog::error("[PeerUDP] Error closing socket: {}", e.what());
    }
}

inline void PeerUDP::send_packet(std::shared_ptr<flatbuffers::DetachedBuffer> packet) {
    co_spawn(ctx_, [this, packet = std::move(packet)]()->boost::asio::awaitable<void> {
        const auto [ec, _] = co_await socket_.async_send_to(
            boost::asio::buffer(packet->data(), packet->size()), remote_endpoint_,
            as_tuple(boost::asio::use_awaitable));

        if (ec) {
            spdlog::error("[PeerUDP] Error sending packet: {}", ec.what());
            close();
        }
    }, boost::asio::detached);
}

inline boost::asio::awaitable<void> PeerUDP::receive_packet() {
    std::vector<uint8_t> buffer(receive_buffer_size_);
    //TODO: Use ring buffer to pack packets without new memory allocations

    if (const auto [ec, _] = co_await socket_.async_receive(
        boost::asio::buffer(buffer.data(), buffer.size()), as_tuple(boost::asio::use_awaitable)); ec) {
        spdlog::error("[PeerUDP] Error receiving packet: {}", ec.what());
        close();
        co_return;
    }
    receive_queue_.push(std::move(buffer));
}
}
