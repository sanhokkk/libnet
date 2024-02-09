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
    static void Register(const std::vector<std::pair<PacketType, PacketFactory>>& factories);
    static std::unique_ptr<Packet> Resolve(PacketType type);

    template <typename T> requires std::is_base_of_v<Packet, T>
    static std::pair<PacketType, PacketFactory> MakePacketFactory(PacketType type);

private:
    inline static std::unordered_map<PacketType, PacketFactory> factory_map_ {};
};


inline void PacketResolver::Register(const std::vector<std::pair<PacketType, PacketFactory>>& factories)
{
    for (const auto& [type, factory] : factories) {
        factory_map_[type] = factory;
    }
}

inline std::unique_ptr<Packet> PacketResolver::Resolve(const PacketType type)
{
    if (!factory_map_.contains(type)) {
        return nullptr;
    }
    return factory_map_[type]();
}

template <typename T> requires std::is_base_of_v<Packet, T>
std::pair<PacketType, PacketFactory> PacketResolver::MakePacketFactory(PacketType type)
{
    return {type, [] { return std::make_unique<T>(); }};
}
}
