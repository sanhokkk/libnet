#pragma once

#include <skymarlin/util/BitConverter.hpp>

#include <span>

namespace skymarlin::util {
class MemoryPool final {
public:
    static bool Initialize() {
        return true;
    }

    static void Uninitialize() {
    }

    static std::span<byte> Allocate(size_t size);

    static void Deallocate(std::span<byte> memory);

private:
    static size_t max_size_;
    static size_t block_size_;
};
}