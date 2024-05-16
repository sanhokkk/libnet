#include <catch2/catch_test_macros.hpp>
#include <skymarlin/concurrent_queue.hpp>

#include <iostream>
#include <thread>

using namespace skymarlin;

TEST_CASE("[ConcurrentQueue]") {
    constexpr size_t ITEM_COUNT = 10000;
    constexpr int ITEM_VALUE = 42;
    ConcurrentQueue<int> queue {};

    SECTION("The consumer busy-waits for the producers") {
        std::thread producer1([&] {
            size_t i {0};
            while (i < ITEM_COUNT / 2) {
                queue.push(ITEM_VALUE);
                ++i;
            }
        });

        std::thread producer2([&] {
            size_t i {0};
            while (i < ITEM_COUNT / 2) {
                queue.push(ITEM_VALUE);
                ++i;
            }
        });

        std::thread consumer([&] {
            size_t i {0};
            while (i < ITEM_COUNT) {
                if (queue.empty())
                    continue;

                auto v = queue.pop();
                REQUIRE(v);
                REQUIRE(*v == ITEM_VALUE);
                ++i;
            }
        });

        producer1.join();
        producer2.join();
        consumer.join();

        REQUIRE(queue.empty());
    }

    SECTION("pop returns std::nullopt from the empty queue") {
        queue.clear();
        REQUIRE(queue.empty());

        const auto v = queue.pop();
        REQUIRE(!v);
    }
}
