#include <catch2/catch_test_macros.hpp>
#include <skymarlin/util/Map.hpp>

#include <thread>

namespace skymarlin::util::test
{
TEST_CASE("Insert/Erase thread safety", "ConcurrentMap")
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

    REQUIRE(map.empty());
}

TEST_CASE("ForEachAll", "ConcurrentMap")
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
    REQUIRE(v1 == 15);
    REQUIRE(v2 == 25);
    REQUIRE(v3 == 35);
}

TEST_CASE("ForEachSome", "ConcurrentMap")
{
    ConcurrentMap<int, int> map {};
    map.InsertOrAssign(1, 10);
    map.InsertOrAssign(2, 21);
    map.InsertOrAssign(3, 30);

    auto odd_numbers = [](const int& n) -> bool {
        return n % 2 != 0;
    };

    int increment = 5;
    map.ForEachSome(std::move(odd_numbers), [](int& value, const int add) {
        value += add;
    }, increment);

    int v1, v2, v3;
    map.TryGet(1, v1);
    map.TryGet(2, v2);
    map.TryGet(3, v3);
    REQUIRE(v1 == 10);
    REQUIRE(v2 == 26);
    REQUIRE(v3 == 30);
}
}
