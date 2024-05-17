#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <skymarlin/concurrent_queue.hpp>

TEST_CASE("[ConcurrentQueue]") {
    using namespace skymarlin;

    BENCHMARK("pop() busy-waits; 1 consumer, 2 producers, 1000000 items") {
        constexpr size_t ITEMS = 1000000;
        constexpr int ITEM_VALUE = 42;
        ConcurrentQueue<int> queue {};

        std::thread consumer([&] {
            for (int i = 0; i < ITEMS; ++i) {
                while (queue.empty()) {}
                const auto v = queue.pop();
                REQUIRE(v);
                REQUIRE(*v == ITEM_VALUE);
            }
        });

        std::thread producer1([&] {
            for (int i = 0; i < ITEMS / 2; ++i) {
                queue.push(ITEM_VALUE);
            }
        });

        std::thread producer2([&] {
            for (int i = 0; i < ITEMS / 2; ++i) {
                queue.push(ITEM_VALUE);
            }
        });

        consumer.join();
        producer1.join();
        producer2.join();
        REQUIRE(queue.empty());
    };

    BENCHMARK("pop_wait() blocks; 1 consumer, 2 producers, 1000000 items") {
        constexpr size_t ITEMS = 1000000;
        constexpr int ITEM_VALUE = 42;
        ConcurrentQueue<int> queue {};

        std::thread consumer([&] {
            for (int i = 0; i < ITEMS; ++i) {
                const auto v = queue.pop_wait();
                REQUIRE(v);
                REQUIRE(*v == ITEM_VALUE);
            }
        });

        std::thread producer1([&] {
            for (int i = 0; i < ITEMS / 2; ++i) {
                queue.push(ITEM_VALUE);
            }
        });

        std::thread producer2([&] {
            for (int i = 0; i < ITEMS / 2; ++i) {
                queue.push(ITEM_VALUE);
            }
        });

        consumer.join();
        producer1.join();
        producer2.join();
        REQUIRE(queue.empty());
    };
}