#pragma once

#include <boost/core/noncopyable.hpp>
#include <skymarlin/util/BitConverter.hpp>

#include <vector>
#include <span>

namespace skymarlin::util {
class Memory final : boost::noncopyable {
public:
    explicit Memory(std::span<byte> memory)
        : memory_(memory) {}

    byte* data() const { return memory_.data(); }
    size_t size() const { return memory_.size(); }

private:
    std::span<byte> memory_;
};
}