#include <catch2/catch_test_macros.hpp>
#include <skymarlin/concurrent_queue.hpp>

#include <iostream>
#include <thread>

using namespace skymarlin;

TEST_CASE("Thread safety", "[ConcurrentQueue]") {
    constexpr int ITEM_COUNT = 10000;
    ConcurrentQueue<int> queue {};

    std::thread t1([&queue] {
        int i {0};
        while (i < ITEM_COUNT) {
            queue.push(42);
            ++i;
        }
    });

    std::thread t2([&queue] {
        int i {0};
        while (i < ITEM_COUNT) {
            if (queue.empty())
                continue;

            queue.pop();
            ++i;
        }
    });

    t1.join();
    t2.join();

    CHECK(queue.empty());
}

TEST_CASE("Pop empty queue", "[ConcurrentQueue]") {
    ConcurrentQueue<int> queue {};
    try {
        queue.pop();
    } catch (const std::out_of_range &e) {
        INFO(std::format("Catch empty queue poping exception: {}", e.what()));
    } catch (const std::exception &e) {
        FAIL("Unexpected exception");
    }
}
