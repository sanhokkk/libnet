#pragma once

#include <mutex>
#include <queue>

namespace skymarlin {
template <typename T>
class ConcurrentQueue {
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue() = default;
    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

    void push(T&& value) {
        std::lock_guard lock {mutex_};

        queue_.push(std::forward<T>(value));
    }

    T pop() {
        std::lock_guard lock {mutex_};

        if (queue_.empty()) throw std::out_of_range("Pop from an empty queue");

        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    bool empty() const {
        std::lock_guard lock {mutex_};

        return queue_.empty();
    }

private:
    std::queue<T> queue_ {};
    mutable std::mutex mutex_ {};
};
}
