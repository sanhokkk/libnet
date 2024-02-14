/*
 * MIT License
 *
 * Copyright (c) 2024 skymarlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <skymarlin/network/Log.hpp>
#include <skymarlin/network/Session.hpp>
#include <skymarlin/thread/Map.hpp>

namespace skymarlin::network
{
class SessionManager
{
public:
    static void AddSession(const std::shared_ptr<Session>& session);
    static void RemoveSession(const std::shared_ptr<Session>& session);
    static void RemoveSession(SessionId id);
    static std::shared_ptr<Session> GetSession(SessionId id);
    static void ClearSessions();

    template<typename... Args>
    static void ForEachSession(std::function<void(std::shared_ptr<Session>&)>&& function, Args&&... args);

private:
    inline static thread::ConcurrentMap<SessionId, std::shared_ptr<Session>> sessions_;
    inline static std::atomic<SessionId> id_generator {0};
};


inline void SessionManager::AddSession(const std::shared_ptr<Session>& session)
{
    session->set_id(++id_generator);
    sessions_.InsertOrAssign(session->id(), session);
}

inline void SessionManager::RemoveSession(const std::shared_ptr<Session>& session)
{
    if (!sessions_.Contains(session->id())) {
        SKYMARLIN_LOG_ERROR("Removing un-added session with id({})", session->id());
        return;
    }
    sessions_.Erase(session->id());
}

inline void SessionManager::RemoveSession(const SessionId id)
{
    if (!sessions_.Contains(id)) {
        SKYMARLIN_LOG_ERROR("Removing un-added session with id({})", id);
        return;
    }
    sessions_.Erase(id);
}

inline std::shared_ptr<Session> SessionManager::GetSession(const SessionId id)
{
    std::shared_ptr<Session> session;
    if (!sessions_.TryGet(id, session)) {
        return nullptr;
    }
    return session;
}

inline void SessionManager::ClearSessions()
{
    sessions_.Clear();
}

template <typename ... Args>
void SessionManager::ForEachSession(std::function<void(std::shared_ptr<Session>&)>&& function, Args&&... args)
{
    sessions_.ForEach(function, args...);
}

}
