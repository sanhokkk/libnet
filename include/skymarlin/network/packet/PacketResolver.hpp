#pragma once

#include <functional>
#include <unordered_map>

namespace skymarlin::network::packet {
using PacketCreator = std::function<std::unique_ptr<Packet>(PacketLength, PacketType)>;
using PacketHandler = std::function<void(std::unique_ptr<Packet>, std::shared_ptr<Session>)>;

template<typename T>
    requires std::same_as<T, PacketCreator> || std::same_as<T, PacketHandler>
class PacketResolver final {
public:
    static void Init(const std::vector<std::pair<PacketType, T>>&& registers) {
        for (const auto& r: registers) {
            map_[r.first] = r.second;
        }
    }

    static bool TryResolve(const PacketType type, T& dest) noexcept {
        if (!map_.contains(type)) {
            return false;
        }
        dest = map_[type];
        return true;
    }

private:
    inline static std::unordered_map<PacketType, T> map_{};
};
}
