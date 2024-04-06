#pragma once

#include <boost/asio/io_context.hpp>
#include <skymarlin/util/Queue.hpp>

namespace skymarlin::util {
template<typename T>
class ConsumerQueue : ConcurrentQueue<T> {
public:
    using ConsumeFunction = std::function<boost::asio::awaitable<const boost::system::error_code>(T&&)>;
    using ErrorHandlingFunction = std::function<void(const boost::system::error_code&)>;

    ConsumerQueue(boost::asio::io_context& io_context, ConsumeFunction&& consume_function,
                  ErrorHandlingFunction&& error_handling_function)
        : io_context_(io_context), consume_function_(std::move(consume_function)),
          error_handling_function_(std::move(error_handling_function)) {}

    void Push(T&& product) {
        ConcurrentQueue<T>::Push(std::move(product));

        if (consuming_.exchange(true)) return;
        co_spawn(io_context_, Consume(), boost::asio::detached);
    }

private:
    boost::asio::awaitable<void> Consume() {
        while (!ConcurrentQueue<T>::empty()) {
            auto product = ConcurrentQueue<T>::Pop();

            if (const auto ec = co_await consume_function_(std::move(product))) {
                error_handling_function_(ec);
                co_return;
            }
        }

        consuming_ = false;
    }

    boost::asio::io_context& io_context_;
    ConsumeFunction consume_function_;
    ErrorHandlingFunction error_handling_function_;
    std::atomic<bool> consuming_ {false};
};
}
