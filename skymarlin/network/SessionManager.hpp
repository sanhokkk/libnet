#pragma once

#include <skymarlin/network/Session.hpp>
#include <skymarlin/thread/Map.hpp>
#include <skymarlin/utility/Log.hpp>

namespace skymarlin::network
{
class SessionManager
{
public:
    static void AddSession(const std::shared_ptr<Session>& session);
    static void RemoveSession(const std::shared_ptr<Session>& session);
    static void RemoveSession(SessionId id);
    static std::shared_ptr<Session> GetSession(SessionId id);

private:
    inline static thread::ConcurrentMap<SessionId, std::shared_ptr<Session>> sessions_;
};


inline void SessionManager::AddSession(const std::shared_ptr<Session>& session)
{
    sessions_[session->id()] = session;
}

inline void SessionManager::RemoveSession(const std::shared_ptr<Session>& session)
{
    if (!sessions_.Contains(session->id())) {
        SKYMARLIN_LOG_ERROR("Removing un-added session with id({})", session->id());
        return;
    }
    sessions_.Erase(session->id());
}

inline void SessionManager::RemoveSession(SessionId id)
{
    if (!sessions_.Contains(id)) {
        SKYMARLIN_LOG_ERROR("Removing un-added session with id({})", id);
        return;
    }
    sessions_.Erase(id);
}

inline std::shared_ptr<Session> SessionManager::GetSession(SessionId id)
{
    if (!sessions_.Contains(id)) {
        return nullptr;
    }
    return sessions_[id];
}
}
