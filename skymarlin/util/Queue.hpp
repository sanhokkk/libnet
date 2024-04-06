#pragma once

#include <mutex>
#include <queue>

namespace skymarlin::util {
template<typename T>
class ConcurrentQueue {
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue() = default;

    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

    void Push(T&& value) {
        std::lock_guard lock(mutex);
        queue_.push(std::forward<T>(value));
    }

    T Pop() {
        std::lock_guard lock(mutex);
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    bool empty() const {
        std::lock_guard lock(mutex);
        return queue_.empty();
    }

private:
    std::queue<T> queue_ {};
    mutable std::mutex mutex {};
};
}
