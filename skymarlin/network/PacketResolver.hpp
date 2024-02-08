#pragma once

#include <functional>
#include <unordered_map>

#include <skymarlin/network/Packet.hpp>

namespace skymarlin::network
{
using PacketFactory = std::function<std::unique_ptr<Packet>()>;

class PacketResolver final
{
public:
    static void Init(const std::vector<std::pair<PacketType, PacketFactory>>& registers);
    static std::unique_ptr<Packet> Resolve(PacketType type);

private:
    inline static std::unordered_map<PacketType, PacketFactory> map_{};
};

inline void PacketResolver::Init(const std::vector<std::pair<PacketType, PacketFactory>>& registers)
{
    for (const auto& [type, creator] : registers) {
        map_[type] = creator;
    }
}

inline std::unique_ptr<Packet> PacketResolver::Resolve(const PacketType type)
{
    if (!map_.contains(type)) {
        return nullptr;
    }
    return map_[type]();
}
}
