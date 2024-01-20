#pragma once

#include <functional>
#include <unordered_map>

namespace skymarlin::network::packet {
class PacketResolver final {
public:
    using PacketCreator = std::function<std::unique_ptr<Packet>(PacketLength, PacketType)>;
    using PacketHandler = std::function<void(std::unique_ptr<Packet>)>;

    PacketResolver() = delete;

    static void Init(const std::vector<std::pair<PacketType, PacketCreator>>& creators,
        const std::vector<std::pair<PacketType, PacketHandler>>& handlers) {
        for (const auto& c: creators) {
            creator_[c.first] = c.second;
        }

        for (const auto& h: handlers) {
            handler_[h.first] = h.second;
        }
    }

    static bool TryGetPacketCreator(const PacketType type, PacketCreator& packet_creator) noexcept {
        if (creator_.contains(type)) {
            packet_creator = creator_[type];
            return true;
        }
        return false;
    }

    static bool TryGetPacketHandler(const PacketType type, PacketHandler& packet_handler) noexcept {
        if (handler_.contains(type)) {
            packet_handler = handler_[type];
            return true;
        }
        return false;
    }

private:
    inline static std::unordered_map<PacketType, PacketCreator> creator_{};
    inline static std::unordered_map<PacketType, PacketHandler> handler_{};
};
}
