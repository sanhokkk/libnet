#pragma once

#include <functional>
#include <unordered_map>

#include <skymarlin/network/Packet.hpp>

namespace skymarlin::network
{
using PacketCreator = std::function<std::shared_ptr<Packet>()>;

class PacketResolver final
{
public:
    static void Init(const std::vector<std::pair<PacketType, PacketCreator>>& registers)
    {
        for (const auto& [type, creator] : registers) {
            map_[type] = creator;
        }
    }

    static std::shared_ptr<Packet> Resolve(const PacketType type)
    {
        if (!map_.contains(type)) {
            return nullptr;
        }
        return map_[type]();
    }

private:
    inline static std::unordered_map<PacketType, PacketCreator> map_{};
};
}
