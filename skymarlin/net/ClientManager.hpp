#pragma once

#include <skymarlin/net/Client.hpp>
#include <skymarlin/util/ConcurrentMap.hpp>

namespace skymarlin::net {
using ClientFactory = std::function<std::unique_ptr<Client>(boost::asio::io_context&, tcp::socket&&, ClientId id)>;

class ClientManager {
public:
    static void Init(ClientFactory client_factory);
    static ClientId CreateClient(boost::asio::io_context& ctx, tcp::socket&& socket);
    static void Broadcast(std::shared_ptr<flatbuffers::DetachedBuffer> message, ClientId except);

    inline static util::ConcurrentMap<ClientId, std::unique_ptr<Client>> clients;

private:
    inline static ClientFactory client_factory_;
    inline static std::atomic<ClientId> id_generator {0};
};

inline void ClientManager::Init(ClientFactory client_factory) {
    client_factory_ = std::move(client_factory);
}

inline ClientId ClientManager::CreateClient(boost::asio::io_context& ctx, tcp::socket&& socket) {
    auto client = client_factory_(ctx, std::move(socket), ++id_generator);
    const auto client_id = client->id();

    client->Start();
    clients.InsertOrAssign(client->id(), std::move(client));

    return client_id;
}

inline void ClientManager::Broadcast(std::shared_ptr<flatbuffers::DetachedBuffer> message, const ClientId except = 0) {
    if (except) {
        clients.ForEachSome(
            [&except](const std::unique_ptr<Client>& client) { return client->id() != except; },
            [](const std::unique_ptr<Client>& client, std::shared_ptr<flatbuffers::DetachedBuffer> message) {
                client->SendMessage(message);
            }, std::move(message));
    } else {
        clients.ForEachAll([](const std::unique_ptr<Client>& client,
            std::shared_ptr<flatbuffers::DetachedBuffer> message) {
                client->SendMessage(message);
            }, std::move(message));
    }
}
}
