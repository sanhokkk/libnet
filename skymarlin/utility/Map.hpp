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

#include <ranges>
#include <shared_mutex>

namespace skymarlin::utility
{
template <typename KeyType, typename ValueType>
class ConcurrentMap
{
public:
    ConcurrentMap() = default;
    ~ConcurrentMap() = default;

    ConcurrentMap(const ConcurrentMap&) = delete;
    ConcurrentMap& operator=(const ConcurrentMap&) = delete;

    void InsertOrAssign(const KeyType& key, const ValueType& value)
    {
        std::unique_lock lock(mutex);
        map_.insert_or_assign(key, value);
    }

    void InsertOrAssign(KeyType&& key, ValueType&& value)
    {
        std::unique_lock lock(mutex);
        map_.insert_or_assign(std::forward<KeyType>(key), std::forward<ValueType>(value));
    }

    bool TryGet(const KeyType& key, ValueType& value) const
    {
        std::shared_lock lock(mutex);

        auto it = map_.find(key);
        if (it == map_.end()) {
            return false;
        }
        value = it->second;
        return true;
    }

    void Erase(const KeyType& key)
    {
        std::unique_lock lock(mutex);
        map_.erase(key);
    }

    void Clear()
    {
        std::unique_lock lock(mutex);
        map_.clear();
    }

    bool Contains(const KeyType& key) const
    {
        std::shared_lock lock(mutex);
        return map_.contains(key);
    }

    template <typename Function, typename... Args> requires std::invocable<Function, ValueType&, Args...>
    void ForEachAll(Function&& function, Args&&... args)
    {
        std::shared_lock lock(mutex);

        for (auto& [_, value] : map_) {
            function(value, args...);
        }
    }

    template <typename Function, typename... Args> requires std::invocable<Function, ValueType&, Args...>
    void ForEachSome(std::function<bool(const ValueType&)>&& filter, Function&& function, Args&&... args)
    {
        std::shared_lock lock(mutex);

        auto view = map_ | std::views::filter(
                 [&filter](const std::pair<KeyType, ValueType>& pair) { return filter(pair.second); });
        for (auto& [_, value] : view) {
            function(value, args...);
        }
    }

    bool empty() const
    {
        std::shared_lock lock(mutex);
        return map_.empty();
    }

private:
    std::unordered_map<KeyType, ValueType> map_ {};
    mutable std::shared_mutex mutex {};
};
}
