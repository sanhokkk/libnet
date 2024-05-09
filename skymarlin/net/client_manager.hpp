#pragma once

#include <skymarlin/concurrent_map.hpp>
#include <skymarlin/net/client.hpp>

namespace skymarlin::net {
using ClientFactory = std::function<std::unique_ptr<Client>(boost::asio::io_context&, tcp::socket&&, ClientId id)>;

class ClientManager {
public:
    static void init(ClientFactory client_factory);
    static ClientId create_client(boost::asio::io_context& ctx, tcp::socket&& socket);
    static void broadcast(std::shared_ptr<flatbuffers::DetachedBuffer> message, ClientId except);

    inline static ConcurrentMap<ClientId, std::unique_ptr<Client>> clients;

private:
    inline static ClientFactory client_factory_;
    inline static std::atomic<ClientId> id_generator {0};
};

inline void ClientManager::init(ClientFactory client_factory) {
    client_factory_ = std::move(client_factory);
}

inline ClientId ClientManager::create_client(boost::asio::io_context& ctx, tcp::socket&& socket) {
    auto client = client_factory_(ctx, std::move(socket), ++id_generator);
    const auto client_id = client->id();

    client->start();
    clients.insert_or_assign(client->id(), std::move(client));

    return client_id;
}

inline void ClientManager::broadcast(std::shared_ptr<flatbuffers::DetachedBuffer> message, const ClientId except = 0) {
    if (except) {
        clients.apply_some(
            [&except](const std::unique_ptr<Client>& client) { return client->id() != except; },
            [](const std::unique_ptr<Client>& client, std::shared_ptr<flatbuffers::DetachedBuffer> message) {
                client->send_message(message);
            }, std::move(message));
    } else {
        clients.apply_all([](const std::unique_ptr<Client>& client,
            std::shared_ptr<flatbuffers::DetachedBuffer> message) {
                client->send_message(message);
            }, std::move(message));
    }
}
}
