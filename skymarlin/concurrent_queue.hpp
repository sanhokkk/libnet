#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace skymarlin {
template <typename T>
class ConcurrentQueue {
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue() = default;
    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

    void push(const T& value);
    void push(T&& value);
    std::optional<T> pop();
    bool empty() const;
    void clear();

private:
    std::queue<T> queue_ {};
    mutable std::mutex mutex_ {};
};

template <typename T>
void ConcurrentQueue<T>::push(const T& value) {
    std::lock_guard lock {mutex_};

    queue_.push(value);
}

template <typename T>
void ConcurrentQueue<T>::push(T&& value) {
    std::lock_guard lock {mutex_};

    queue_.push(std::forward<T>(value));
}

template <typename T>
std::optional<T> ConcurrentQueue<T>::pop() {
    std::lock_guard lock {mutex_};

    if (queue_.empty()) return std::nullopt;

    T value = std::move(queue_.front());
    queue_.pop();
    return std::make_optional(std::move(value));
}

template <typename T>
bool ConcurrentQueue<T>::empty() const {
    std::lock_guard lock {mutex_};

    return queue_.empty();
}

template <typename T>
void ConcurrentQueue<T>::clear() {
    std::lock_guard lock {mutex_};

    queue_ = {};
}
}
