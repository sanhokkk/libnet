#pragma once

#include <skymarlin/net/Client.hpp>
#include <skymarlin/util/ConcurrentMap.hpp>

namespace skymarlin::net {
using ClientFactory = std::function<std::shared_ptr<Client>(boost::asio::io_context&, tcp::socket&&, ClientId id)>;

class ClientManager {
public:
    static void Init(ClientFactory client_factory);
    static std::shared_ptr<Client> CreateClient(boost::asio::io_context& ctx, tcp::socket&& socket);
    static void Broadcast(std::shared_ptr<flatbuffers::DetachedBuffer> message,
        std::shared_ptr<Client> except);

    inline static util::ConcurrentMap<ClientId, std::shared_ptr<Client>> clients;

private:
    inline static ClientFactory client_factory_;
    inline static std::atomic<ClientId> id_generator {0};
};

inline void ClientManager::Init(ClientFactory client_factory) {
    client_factory_ = std::move(client_factory);
}

inline std::shared_ptr<Client> ClientManager::CreateClient(boost::asio::io_context& ctx, tcp::socket&& socket) {
    auto client = client_factory_(ctx, std::move(socket), ++id_generator);
    clients.InsertOrAssign(client->id(), client);
    return client;
}

inline void ClientManager::Broadcast(std::shared_ptr<flatbuffers::DetachedBuffer> message,
    std::shared_ptr<Client> except = nullptr) {
    if (except) {
        clients.ForEachSome(
            [&except](const std::shared_ptr<Client>& client) { return client == except; },
            [](const std::shared_ptr<Client>& client, std::shared_ptr<flatbuffers::DetachedBuffer> message) {
                client->SendMessage(message);
            }, std::move(message));
    } else {
        clients.ForEachAll([](const std::shared_ptr<Client>& client,
            std::shared_ptr<flatbuffers::DetachedBuffer> message) {
                client->SendMessage(message);
            }, std::move(message));
    }
}
}
