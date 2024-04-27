#pragma once

#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace skymarlin::util {
template <typename KeyType, typename ValueType>
class ConcurrentMap {
public:
    ConcurrentMap() = default;
    ~ConcurrentMap() = default;
    ConcurrentMap(const ConcurrentMap&) = delete;
    ConcurrentMap& operator=(const ConcurrentMap&) = delete;

    void InsertOrAssign(const KeyType& key, const ValueType& value) {
        std::unique_lock lock {mutex_};
        map_.insert_or_assign(key, value);
    }

    void InsertOrAssign(KeyType&& key, ValueType&& value) {
        std::unique_lock lock {mutex_};
        map_.insert_or_assign(std::forward<KeyType>(key), std::forward<ValueType>(value));
    }

    bool TryGet(const KeyType& key, ValueType& value) const {
        std::shared_lock lock {mutex_};

        auto it = map_.find(key);
        if (it == map_.end()) {
            return false;
        }
        value = it->second;
        return true;
    }

    void Erase(const KeyType& key) {
        std::unique_lock lock {mutex_};
        map_.erase(key);
    }

    void Clear() {
        std::unique_lock lock {mutex_};
        map_.clear();
    }

    bool Contains(const KeyType& key) const {
        std::shared_lock lock {mutex_};
        return map_.contains(key);
    }

    template <typename Function, typename... Args> requires std::invocable<Function, ValueType&, Args...>
    void ForEachAll(Function function, Args... args) {
        std::shared_lock lock {mutex_};

        for (auto& [_, value] : map_) {
            function(value, args...);
        }
    }

    template <typename Filter, typename Function, typename... Args>
        requires std::invocable<Filter, const ValueType&>
        && std::same_as<bool, std::invoke_result_t<Filter, const ValueType&>>
        && std::invocable<Function, ValueType&, Args...>
    void ForEachSome(Filter filter, Function function, Args... args) {
        std::shared_lock lock {mutex_};

        for (auto& [_, value] : map_) {
            if (!filter(value)) continue;

            function(value, args...);
        }
    }

    bool empty() const {
        std::shared_lock lock {mutex_};
        return map_.empty();
    }

private:
    std::unordered_map<KeyType, ValueType> map_ {};
    mutable std::shared_mutex mutex_ {};
};
}
