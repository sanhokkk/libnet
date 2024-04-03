#pragma once

#include <boost/asio.hpp>
#include <flatbuffers/detached_buffer.h>
#include <skymarlin/net/IConnection.hpp>
#include <skymarlin/net/Message.hpp>
#include <skymarlin/net/MessageHandler.hpp>
#include <skymarlin/util/Log.hpp>
#include <skymarlin/util/Queue.hpp>

namespace skymarlin::net {
using boost::asio::ip::tcp;

class Connection final : public IConnection, public std::enable_shared_from_this<Connection> {
public:
    Connection(boost::asio::io_context& io_context, tcp::socket&& socket);
    ~Connection() override;

    void Disconnect() override;
    void StartReceiveMessage() override;
    void SendMessage(std::shared_ptr<flatbuffers::DetachedBuffer> message) override;

    static boost::asio::awaitable<bool> Connect(Connection& connection, std::string_view host, uint16_t port);

    bool connected() const override { return connected_; }
    tcp::endpoint local_endpoint() const override { return socket_.lowest_layer().local_endpoint(); }
    tcp::endpoint remote_endpoint() const override { return socket_.lowest_layer().remote_endpoint(); }

private:
    boost::asio::awaitable<std::unique_ptr<Message>> ReceiveMessage();
    boost::asio::awaitable<std::optional<MessageHeader>> ReadMessageHeader();
    boost::asio::awaitable<std::optional<std::vector<byte>>> ReadMessageBody(MessageHeader header);
    boost::asio::awaitable<void> ProcessSendQueue();

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    std::atomic<bool> connected_ {true};

    util::ConcurrentQueue<std::shared_ptr<flatbuffers::DetachedBuffer>> send_queue_ {};
    std::atomic<bool> send_queue_processing_ {false};
};


inline Connection::Connection(boost::asio::io_context& io_context, tcp::socket&& socket)
    : io_context_(io_context), socket_(std::move(socket)) {}

inline Connection::~Connection() {
    if (connected_) {
        SKYMARLIN_LOG_WARN("Connection destructing without being disconnected");
    }
}

inline void Connection::StartReceiveMessage() {
    if (!connected_) return;

    co_spawn(io_context_, [this]()-> boost::asio::awaitable<void> {
        while (connected_) {
            auto message = co_await ReceiveMessage();
            if (!message) continue;

            MessageHandler::Handle(std::move(message), shared_from_this());
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
        SKYMARLIN_LOG_ERROR("Error on shutdwon socket: {}", e.what());
    }
}

inline void Connection::SendMessage(std::shared_ptr<flatbuffers::DetachedBuffer> message) {
    if (!message || !connected_) return;

    send_queue_.Push(std::move(message));

    if (send_queue_processing_.exchange(true)) return;
    co_spawn(io_context_, ProcessSendQueue(), boost::asio::detached);
}

inline boost::asio::awaitable<bool> Connection::Connect(Connection& connection, std::string_view host, uint16_t port) {
    tcp::resolver resolver {connection.io_context_};

    SKYMARLIN_LOG_INFO("Trying to connect to {}:{}", host, port);
    const auto [ec, endpoints] = co_await resolver.async_resolve(host,
                                                                 std::format("{}", port),
                                                                 as_tuple(boost::asio::use_awaitable));
    if (ec) {
        SKYMARLIN_LOG_ERROR("Error on resolve: {}", ec.what());
        connection.connected_ = false;
        co_return false;
    }

    if (const auto [ec, endpoint] = co_await async_connect(connection.socket_, endpoints,
                                                           as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on connect: {}", ec.what());
        connection.connected_ = false;
        co_return false;
    }

    SKYMARLIN_LOG_INFO("Connected to {}:{}", host, port);

    co_return true;
}

inline boost::asio::awaitable<void> Connection::ProcessSendQueue() {
    while (!send_queue_.empty()) {
        const auto message = send_queue_.Pop();

        if (const auto [ec, _] = co_await async_write(socket_,
                                                      boost::asio::buffer(message->data(), message->size()),
                                                      as_tuple(boost::asio::use_awaitable)); ec) {
            SKYMARLIN_LOG_ERROR("Error on sending packet: {}", ec.what());
            Disconnect();
            co_return;
        }
    }

    send_queue_processing_ = false;
}

inline boost::asio::awaitable<std::unique_ptr<Message>> Connection::ReceiveMessage() {
    const std::optional<MessageHeader> header = co_await ReadMessageHeader();
    if (!header) {
        Disconnect();
        co_return nullptr;
    }

    auto buffer = co_await ReadMessageBody(*header);
    if (!buffer) {
        Disconnect();
        co_return nullptr;
    }

    co_return std::make_unique<Message>(std::move(*buffer), *header);
}

inline boost::asio::awaitable<std::optional<MessageHeader>> Connection::ReadMessageHeader() {
    std::array<byte, MessageHeader::HEADER_SIZE> buffer {};

    if (const auto [ec, _] = co_await async_read(socket_,
                                                 boost::asio::buffer(buffer),
                                                 as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet header: {}", ec.what());
        co_return std::nullopt;
    }

    co_return MessageHeader::ReadHeader(buffer);
}

inline boost::asio::awaitable<std::optional<std::vector<byte>>> Connection::ReadMessageBody(
    const MessageHeader header) {
    //TODO: Alllocate buffer from the memory pool
    std::vector<byte> buffer(header.size);

    if (const auto [ec, _] = co_await async_read(socket_,
                                                 boost::asio::buffer(buffer),
                                                 as_tuple(boost::asio::use_awaitable)); ec) {
        SKYMARLIN_LOG_ERROR("Error on receiving packet body: {}", ec.what());
        co_return std::nullopt;
    }

    co_return buffer;
}
}
