#pragma once

#include <functional>
#include <unordered_map>

namespace skymarlin::network::packet {
using PacketCreator = std::function<std::unique_ptr<Packet>()>;

class PacketResolver final {

public:
    static void Init(const std::vector<std::pair<PacketType, PacketCreator>>&& registers) {
        for (const auto& [type, creator]: registers) {
            map_[type] = creator;
        }
    }

    static bool TryResolve(const PacketType type, std::unique_ptr<Packet>& dest) {
        if (!map_.contains(type)) {
            return false;
        }
        dest = map_[type]();
        return true;
    }

private:
    inline static std::unordered_map<PacketType, PacketCreator> map_{};
};
}
