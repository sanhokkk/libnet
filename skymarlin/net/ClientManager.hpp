#pragma once

#include <skymarlin/net/Client.hpp>
#include <skymarlin/util/ConcurrentMap.hpp>

namespace skymarlin::net {
using ClientFactory = std::function<std::shared_ptr<Client>(boost::asio::io_context&, tcp::socket&&, ClientId id)>;

class ClientManager {
public:
    static void Init(ClientFactory client_factory);
    static std::shared_ptr<Client> CreateClient(boost::asio::io_context& io_context, tcp::socket&& socket);

    inline static util::ConcurrentMap<ClientId, std::shared_ptr<Client>> clients;

private:
    inline static ClientFactory client_factory_;
    inline static std::atomic<ClientId> id_generator {0};
};

inline void ClientManager::Init(ClientFactory client_factory) {
    client_factory_ = std::move(client_factory);
}

inline std::shared_ptr<Client> ClientManager::CreateClient(boost::asio::io_context& io_context, tcp::socket&& socket) {
    auto client = client_factory_(io_context, std::move(socket), ++id_generator);
    clients.InsertOrAssign(client->id(), client);
    return client;
}
}
