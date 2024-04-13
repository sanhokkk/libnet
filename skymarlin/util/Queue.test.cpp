#include <catch2/catch_test_macros.hpp>
#include <skymarlin/util/Queue.hpp>

#include <thread>

namespace skymarlin::util::test
{
TEST_CASE("Thread safety", "[ConcurrentQueue]")
{
    ConcurrentQueue<int> queue {};

    std::thread t1([&queue] {
        for (int i = 0; i < 10000; ++i) {
            queue.Push(42);
            queue.Pop();
        }
    });

    std::thread t2([&queue] {
        for (int i = 0; i < 10000; ++i) {
            queue.Push(27);
            queue.Pop();
        }
    });

    t1.join();
    t2.join();

    REQUIRE(queue.empty());
}
}
