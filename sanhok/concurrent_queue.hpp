#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace sanhok {
/*
 * A thread-safe queue for single consumer and mutliple producers
 */
template <typename T>
class ConcurrentQueue {
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue();
    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

    void push(const T& value);
    void push(T&& value);
    std::optional<T> pop();
    std::optional<T> pop_wait();
    bool empty() const;
    void clear();

private:
    std::queue<T> queue_ {};
    mutable std::mutex mutex_ {};
    std::condition_variable cv_ {};
};

template <typename T>
ConcurrentQueue<T>::~ConcurrentQueue() {
    clear();
}

template <typename T>
void ConcurrentQueue<T>::push(const T& value) {
    std::unique_lock lock {mutex_};

    const bool was_empty = queue_.empty();
    queue_.push(value);
    if (was_empty) {
        lock.unlock();
        cv_.notify_one();
    }
}

template <typename T>
void ConcurrentQueue<T>::push(T&& value) {
    std::unique_lock lock {mutex_};

    const bool was_empty = queue_.empty();
    queue_.push(std::forward<T>(value));
    if (was_empty) {
        lock.unlock();
        cv_.notify_one();
    }
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
std::optional<T> ConcurrentQueue<T>::pop_wait() {
    std::unique_lock lock {mutex_};

    if (queue_.empty()) cv_.wait(lock);
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
    std::unique_lock lock {mutex_};

    queue_ = {};
    lock.unlock();
    cv_.notify_all();
}
}
