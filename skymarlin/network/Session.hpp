#pragma once

#include <coroutine>
#include <memory>

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <skymarlin/network/Packet.hpp>
#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/thread/Queue.hpp>
#include <skymarlin/utility/ByteBufferExceptions.hpp>
#include <skymarlin/utility/Log.hpp>
#include <skymarlin/utility/MutableByteBuffer.hpp>

namespace skymarlin::network
{
using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>, boost::noncopyable
{
public:
    explicit Session(tcp::socket&& socket);

    virtual ~Session();

    void Open();
    void Close();
    void Write(boost::asio::const_buffer& buffer);

    [[nodiscard]] bool open() const { return !closed_ && !closing_; };
    tcp::endpoint local_endpoint() const { return socket_.local_endpoint(); }
    tcp::endpoint remote_endpoint() const { return socket_.remote_endpoint(); }

protected:
    virtual void OnClose() = 0;

private:
    struct AwaitableReceive;

    AwaitableReceive ReceivePacket();
    void ReadHeader();
    void OnReadHeader();
    void OnReadPacket(PacketType packet_type, const boost::asio::mutable_buffer& buffer);

    void WriteInternal();

    tcp::socket socket_;
    boost::asio::streambuf read_streambuf_;
    boost::asio::streambuf write_streambuf_;
    byte header_buffer_source_[PACKET_HEADER_SIZE] {};
    utility::ConstByteBuffer header_buffer_;

    thread::ConcurrentQueue<boost::asio::const_buffer> write_queue_;
    std::atomic<bool> writing_ {false};

    std::atomic<bool> closed_, closing_;
};

struct Session::AwaitableReceive : boost::noncopyable
{
    struct promise_type; // using promise_type = YouCanExtractPromiseTypeLikeThis;
    // void resume() { handle_.resume(); }

    AwaitableReceive(AwaitableReceive&& other) noexcept
        : handle_(other.handle_) { other.handle_ = nullptr; }

    AwaitableReceive& operator=(AwaitableReceive&& other) noexcept
    {
        if (handle_) handle_.destroy();
        handle_ = other.handle_;
        other.handle_ = nullptr;

        return *this;
    }

    ~AwaitableReceive() { if (handle_) handle_.destroy(); }

    std::optional<std::unique_ptr<Packet>> next() {}

private:
    using handle_type = std::coroutine_handle<promise_type>;

    explicit AwaitableReceive(handle_type handle) : handle_(handle) {}

    handle_type handle_;

    struct promise_type
    {
        std::unique_ptr<Packet> packet;

        AwaitableReceive get_return_object()
        {
            const auto handle = handle_type::from_promise(*this);
            return AwaitableReceive {handle};
        }

        void return_void() {}

        std::suspend_never yield_value(std::unique_ptr<Packet> packet)
        {
            this->packet = std::move(packet);
            return {};
        }

        // std::suspend_never await_transform(Receive) { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }

        void unhandled_exception() {}
    };
};

inline Session::Session(tcp::socket&& socket)
    : socket_(std::move(socket)), header_buffer_(header_buffer_source_, PACKET_HEADER_SIZE),
    closed_(false), closing_(false)
{
    try {
        socket_.set_option(tcp::no_delay(true));
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error setting socket no-delay: {}", e.what());
    }
}


inline Session::~Session()
{
    closed_ = true;

    try {
        socket_.close();
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing socket: {}", e.what());
    }
}

inline void Session::Open()
{
    std::thread([this] {
        AwaitableReceive r = ReceivePacket();
        while (auto packet = r.next()) {}

        SKYMARLIN_LOG_INFO("Session receive thread terminating");
    }).detach();

    std::thread([this] {
        AwaitableSend s = ;
        while (auto packet = s.next()) {
            // send packet

            if (closing_ && packet == last_packet) closed_ = true;
        }

        SKYMARLIN_LOG_INFO("Session send thread terminating");
    }).detach();
}

inline void Session::Close()
{
    if (closed_.exchange(true)) return;

    try {
        socket_.shutdown(tcp::socket::shutdown_send);
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error closing socket: {}", e.what());
    }

    OnClose();
}

inline Session::AwaitableReceive Session::ReceivePacket()
{
    try {
        // 1. Wait for packet header read
        co_await ReadPacketHeader(); // suspend

        // 2. Get packet length, type
        size_t length = ;
        PacketType type = ;

        // 3. Wait for packet body read
        auto buffer = ReadPacketBody();

        // 4. Create packet
        auto packet = PacketResolver::Resolve(type);
        packet->Deserialize(buffer);

        // 5. Handle packet
        packet->Handle(this);
    }
    catch (const boost::system::system_error& e) {
        SKYMARLIN_LOG_ERROR("Error reading packet: {}", e.what());
        Close();
        co_return;
    }
    catch (const skymarlin::utility::ByteBufferException& e) {
        SKYMARLIN_LOG_ERROR("Error deserializing packet: {}", e.what());
        Close();
        co_return;
    }
}


inline void Session::ReadHeader()
{
    if (!open())
        return;

    boost::asio::async_read(socket_,
        boost::asio::buffer(header_buffer_source_),
        [self = shared_from_this()](const boost::system::error_code& ec,
        const size_t bytes_transferred) {
            if (ec) {
                SKYMARLIN_LOG_ERROR("Error reading packet header: {}", ec.what());
                self->Stop();
                return;
            }
            if (bytes_transferred == 0) {
                SKYMARLIN_LOG_ERROR("Error reading packet header: zero bytes read for packet header");
                self->Stop();
                return;
            }

            self->OnReadHeader();
        });
}

inline void Session::OnReadHeader()
{
    PacketLength packet_length;
    PacketType packet_type;
    try {
        header_buffer_ >> packet_length >> packet_type;
    }
    catch (const utility::ByteBufferException& e) {
        SKYMARLIN_LOG_ERROR("Error reading bytes on header: {}", e.what());
        Stop();
        return;
    }
    catch (const std::exception& e) {
        SKYMARLIN_LOG_ERROR(e.what());
        Stop();
        return;
    }

    std::shared_ptr<boost::asio::mutable_buffer> buffer;
    try {
        buffer = std::make_shared<boost::asio::mutable_buffer>(read_streambuf_.prepare(packet_length));
    }
    catch (const std::length_error& e) {
        SKYMARLIN_LOG_ERROR("Insuffcient buffer size to prepare for packet body: {}", e.what());
        Stop();
        return;
    } catch (const std::exception& e) {
        SKYMARLIN_LOG_ERROR(e.what());
        Stop();
        return;
    }

    boost::asio::async_read(socket_,
        *buffer,
        boost::asio::transfer_exactly(packet_length),
        [self = shared_from_this(), packet_length, packet_type, buffer](
        const boost::system::error_code& ec, const size_t bytes_transferred) {
            if (ec) {
                SKYMARLIN_LOG_ERROR("Error reading packet body: : {}", ec.message());
                self->Stop();
                return;
            }
            if (bytes_transferred != packet_length) {
                SKYMARLIN_LOG_ERROR(
                    "Error reading packet body: Inconsistent bytes between read bytes and packet length");
                self->Stop();
                return;
            }

            self->OnReadPacket(packet_type, *buffer);
            self->read_streambuf_.consume(bytes_transferred);
        });

    ReadHeader();
}

inline void Session::OnReadPacket(const PacketType packet_type, const boost::asio::mutable_buffer& buffer)
{
    const std::shared_ptr<Packet> packet = PacketResolver::Resolve(packet_type);
    if (!packet) {
        SKYMARLIN_LOG_ERROR("Invalid packet type: {}", packet_type);
        Stop();
        return;
    }

    packet->Deserialize(buffer);
    packet->Handle(shared_from_this());
}

inline void Session::Write(boost::asio::const_buffer& buffer)
{
    write_queue_.Push(std::move(buffer));

    if (writing_.exchange(true))
        return;

    // TODO: Extract? & Lauch as async
    while (!write_queue_.Empty()) {
        boost::asio::async_write(socket_, write_queue_.Pop(),
            [self = shared_from_this()](const boost::system::error_code& ec,
            const size_t bytes_transferred) {
                if (ec) {
                    SKYMARLIN_LOG_ERROR("Error writing packet header: {}", ec.message());
                    self->Stop();
                    return;
                }
                if (bytes_transferred == 0) {
                    SKYMARLIN_LOG_ERROR("Error writing packet header: zero bytes written for packet header");
                    self->Stop();
                    return;
                }
            });
    }
    writing_ = false;
}
}
