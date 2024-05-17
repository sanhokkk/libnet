#include <catch2/catch_test_macros.hpp>
#include <skymarlin/concurrent_queue.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace skymarlin;
using namespace std::chrono_literals;

TEST_CASE("[ConcurrentQueue]") {
    constexpr size_t ITEM_COUNT = 10000;
    constexpr int ITEM_VALUE = 42;
    ConcurrentQueue<int> queue {};

    SECTION("pop returns std::nullopt from the empty queue") {
        queue.clear();

        const auto v = queue.pop();
        REQUIRE(v == std::nullopt);
    }

    SECTION("The consumer busy-waits for the producers") {
        queue.clear();

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

        consumer.join();
        producer1.join();
        producer2.join();

        REQUIRE(queue.empty());
    }

    SECTION("pop_wait returns std::nullopt after clear") {
        queue.clear();

        std::atomic<bool> is_returned {false};
        std::thread consumer([&] {
            auto v = queue.pop_wait();
            REQUIRE(v == std::nullopt);
            is_returned = true;
        });

        while (!is_returned) queue.clear();
        REQUIRE(queue.empty());

        consumer.join();
    }

    SECTION("The consumer waits for the producers with condition variable") {
        queue.clear();

        std::thread consumer([&] {
            size_t i {0};
            while (i < ITEM_COUNT) {
                auto v = queue.pop_wait();
                REQUIRE(v);
                REQUIRE(*v == ITEM_VALUE);
                ++i;
            }
        });

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

        consumer.join();
        producer1.join();
        producer2.join();

        REQUIRE(queue.empty());
    }
}
