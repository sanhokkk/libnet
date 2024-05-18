#include <catch2/catch_test_macros.hpp>
#include <sanhok/concurrent_map.hpp>

#include <thread>

using namespace sanhok;

TEST_CASE("Insert/Erase thread safety", "[ConcurrentMap]")
{
    ConcurrentMap<int, int> map {};

    std::thread t1([&map] {
        for (int i = 0; i < 10000; ++i) {
            map.insert_or_assign(i, 42);
            map.erase(i);
        }
    });

    std::thread t2([&map] {
        for (int i = 10000; i < 20000; ++i) {
            map.insert_or_assign(i, 27);
            map.erase(i);
        }
    });

    t1.join();
    t2.join();

    REQUIRE(map.empty());
}

TEST_CASE("apply", "[ConcurrentMap]")
{
    ConcurrentMap<int, int> map {};
    map.insert_or_assign(2, 20);

    map.apply(2, [](int& value, const int add) {
        value += add;
    }, 10);

    REQUIRE(map.at(2) == 30);
}

TEST_CASE("apply_all", "[ConcurrentMap]")
{
    ConcurrentMap<int, int> map {};
    map.insert_or_assign(1, 10);
    map.insert_or_assign(2, 20);
    map.insert_or_assign(3, 30);

    int increment = 5;
    map.apply_all([](int& value, const int add) {
        value += add;
    }, increment);

    REQUIRE(map.at(1) == 15);
    REQUIRE(map.at(2) == 25);
    REQUIRE(map.at(3) == 35);
}

TEST_CASE("apply_some", "[ConcurrentMap]")
{
    ConcurrentMap<int, int> map {};
    map.insert_or_assign(1, 10);
    map.insert_or_assign(2, 21);
    map.insert_or_assign(3, 30);

    auto odd_numbers = [](const int& n) -> bool {
        return n % 2 != 0;
    };

    int increment = 5;
    map.apply_some(std::move(odd_numbers), [](int& value, const int add) {
        value += add;
    }, increment);

    REQUIRE(map.at(1) == 10);
    REQUIRE(map.at(2) == 26);
    REQUIRE(map.at(3) == 30);
}

