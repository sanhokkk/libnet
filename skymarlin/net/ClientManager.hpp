#pragma once

#include <skymarlin/net/Client.hpp>
#include <skymarlin/util/Log.hpp>
#include <skymarlin/util/Map.hpp>

namespace skymarlin::net
{
class ClientManager
{
public:
    static void AddClient(const std::shared_ptr<Client>& client);
    static void RemoveClient(const std::shared_ptr<Client>& session);
    static void RemoveClient(ClientId id);
    static std::shared_ptr<Client> GetClient(ClientId id);
    static void ClearClients();

    template <typename Function, typename... Args> requires std::invocable<Function, std::shared_ptr<Client>&, Args...>
    static void ForEachAllClient(Function&& function, Args&&... args);
    template <typename Filter, typename Function, typename... Args>
        requires std::invocable<Filter, const std::shared_ptr<Client>&>
        && std::same_as<bool, std::invoke_result_t<Filter, const std::shared_ptr<Client>&>>
        && std::invocable<Function, std::shared_ptr<Client>&, Args...>
    static void ForEachSomeClient(Filter&& filter, Function&& function, Args&&... args);

private:
    inline static util::ConcurrentMap<ClientId, std::shared_ptr<Client>> clients_;
    inline static std::atomic<ClientId> id_generator {0};
};


inline void ClientManager::AddClient(const std::shared_ptr<Client>& client)
{
    clients_.InsertOrAssign(client->id(), client);
}

inline void ClientManager::RemoveClient(const std::shared_ptr<Client>& session)
{
    if (!clients_.Contains(session->id())) {
        SKYMARLIN_LOG_ERROR("Removing un-added session with id({})", session->id());
        return;
    }
    clients_.Erase(session->id());
}

inline void ClientManager::RemoveClient(const ClientId id)
{
    if (!clients_.Contains(id)) {
        SKYMARLIN_LOG_ERROR("Removing un-added session with id({})", id);
        return;
    }
    clients_.Erase(id);
}

inline std::shared_ptr<Client> ClientManager::GetClient(const ClientId id)
{
    std::shared_ptr<Client> session;
    if (!clients_.TryGet(id, session)) {
        return nullptr;
    }
    return session;
}

inline void ClientManager::ClearClients()
{
    clients_.Clear();
}

template <typename Function, typename... Args> requires std::invocable<Function, std::shared_ptr<Client>&, Args...>
void ClientManager::ForEachAllClient(Function&& function, Args&&... args)
{
    clients_.ForEachAll(std::forward<Function>(function), std::forward<Args>(args)...);
}

template <typename Filter, typename Function, typename... Args>
    requires std::invocable<Filter, const std::shared_ptr<Client>&>
    && std::same_as<bool, std::invoke_result_t<Filter, const std::shared_ptr<Client>&>>
    && std::invocable<Function, std::shared_ptr<Client>&, Args...>
void ClientManager::ForEachSomeClient(Filter&& filter, Function&& function, Args&&... args)
{
    clients_.ForEachSome(std::forward<Filter>(filter), std::forward<Function>(function), std::forward<Args>(args)...);
}
}