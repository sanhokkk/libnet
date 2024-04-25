#pragma once

#include <boost/asio.hpp>
#include <flatbuffers/flatbuffers.h>
#include <skymarlin/util/ConcurrentQueue.hpp>
#include <spdlog/spdlog.h>

namespace skymarlin::net {
using boost::asio::ip::tcp;

class Connection final : public std::enable_shared_from_this<Connection> {
public:
    Connection(boost::asio::io_context& io_context, tcp::socket&& socket,
        util::ConcurrentQueue<std::vector<uint8_t>>& receive_queue);
    ~Connection();

    void Start();
    void Disconnect();
    void SendMessage(std::shared_ptr<flatbuffers::DetachedBuffer> message);

    bool connected() const { return connected_; }
    tcp::endpoint local_endpoint() const { return socket_.lowest_layer().local_endpoint(); }
    tcp::endpoint remote_endpoint() const { return socket_.lowest_layer().remote_endpoint(); }

private:
    boost::asio::awaitable<std::optional<std::vector<uint8_t>>> ReceiveMessage();
    boost::asio::awaitable<void> ProcessSendQueue();

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    std::atomic<bool> connected_ {true};

    util::ConcurrentQueue<std::vector<uint8_t>>& receive_queue_;
    util::ConcurrentQueue<std::shared_ptr<flatbuffers::DetachedBuffer>> send_queue_ {};
};


inline Connection::Connection(boost::asio::io_context& io_context, tcp::socket&& socket,
    util::ConcurrentQueue<std::vector<uint8_t>>& receive_queue)
    : io_context_(io_context), socket_(std::move(socket)), receive_queue_(receive_queue) {}

inline Connection::~Connection() {
    if (connected_) {
        spdlog::warn("[Connection] Destructor called while connected");
    }
}

inline void Connection::Start() {
    if (!connected_) return;

    co_spawn(io_context_, [this]()-> boost::asio::awaitable<void> {
        while (connected_) {
            auto message = co_await ReceiveMessage();
            if (!message) continue;

            receive_queue_.Push(std::move(*message));
        }
    }, boost::asio::detached);

    co_spawn(io_context_, [this]()-> boost::asio::awaitable<void> {
        while (connected_) {
            const auto message = send_queue_.Pop();
            if (!message) continue;

            if (auto [ec, _] = co_await async_write(socket_,
                boost::asio::buffer(message->data(), message->size()),
                as_tuple(boost::asio::use_awaitable)); ec) {
                spdlog::error("[Connection] Error sending message: {}", ec.what());
                Disconnect();
                co_return;
            }
        }
    }, boost::asio::detached);
}

inline void Connection::Disconnect() {
    if (!connected_.exchange(false)) return;

    try {
        socket_.shutdown(tcp::socket::shutdown_both);

        //TODO: Process remaining send queue?
        socket_.close();
    } catch (const boost::system::system_error& e) {
        spdlog::error("[Connection] Error shutting down socket: {}", e.what());
    }
}

inline void Connection::SendMessage(std::shared_ptr<flatbuffers::DetachedBuffer> message) {
    if (!message || !connected_) return;

    send_queue_.Push(std::move(message));
}

inline boost::asio::awaitable<std::optional<std::vector<uint8_t>>> Connection::ReceiveMessage() {
    constexpr size_t MESSAGE_SIZE_PREFIX_BYTES = sizeof(flatbuffers::uoffset_t);

    std::array<uint8_t, MESSAGE_SIZE_PREFIX_BYTES> header_buffer {};
    if (const auto [ec, _] = co_await async_read(socket_,
        boost::asio::buffer(header_buffer),
        as_tuple(boost::asio::use_awaitable)); ec) {
        Disconnect();
        co_return std::nullopt;
    }

    const auto length = flatbuffers::GetSizePrefixedBufferLength(header_buffer.data());
    //TODO: Managed memory allocation
    std::vector<uint8_t> body_buffer(length - MESSAGE_SIZE_PREFIX_BYTES);

    if (const auto [ec, _] = co_await async_read(socket_,
        boost::asio::buffer(body_buffer),
        as_tuple(boost::asio::use_awaitable)); ec) {
        Disconnect();
        co_return std::nullopt;
    }
    co_return body_buffer;
}
}
