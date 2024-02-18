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

#include <gtest/gtest.h>
#include <skymarlin/utility/Map.hpp>

namespace skymarlin::utility::test
{
TEST(ConcurrentMap, ThreadSafety)
{
    ConcurrentMap<int, int> map {};

    std::thread t1([&map] {
        for (int i = 0; i < 10000; ++i) {
            map.InsertOrAssign(i, 42);
            map.Erase(i);
        }
    });

    std::thread t2([&map] {
        for (int i = 10000; i < 20000; ++i) {
            map.InsertOrAssign(i, 27);
            map.Erase(i);
        }
    });

    t1.join();
    t2.join();

    if (!map.empty()) {
        FAIL();
    }
}

TEST(ConcurrentMap, ForEachAll)
{
    ConcurrentMap<int, int> map {};
    map.InsertOrAssign(1, 10);
    map.InsertOrAssign(2, 20);
    map.InsertOrAssign(3, 30);

    int increment = 5;
    map.ForEachAll([](int& value, const int add) {
        value += add;
    }, increment);

    int v1, v2, v3;
    map.TryGet(1, v1);
    map.TryGet(2, v2);
    map.TryGet(3, v3);
    ASSERT_EQ(v1, 15);
    ASSERT_EQ(v2, 25);
    ASSERT_EQ(v3, 35);
}

TEST(ConcurrentMap, ForEachSome)
{
    ConcurrentMap<int, int> map {};
    map.InsertOrAssign(1, 10);
    map.InsertOrAssign(2, 21);
    map.InsertOrAssign(3, 30);

    auto odd_numbers = [](const int& n) {
        return n % 2 != 0;
    };

    int increment = 5;
    map.ForEachSome(std::move(odd_numbers), [] (int& value, const int add) {
        value += add;
    }, increment);

    int v1, v2, v3;
    map.TryGet(1, v1);
    map.TryGet(2, v2);
    map.TryGet(3, v3);
    ASSERT_EQ(v1, 10);
    ASSERT_EQ(v2, 26);
    ASSERT_EQ(v3, 30);
}
}
