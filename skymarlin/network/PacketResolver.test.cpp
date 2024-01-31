#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <skymarlin/network/Packet.hpp>
#include <skymarlin/network/PacketResolver.hpp>

namespace skymarlin::network
{
class TestPacket final : public Packet
{
public:
    void Serialize(boost::asio::mutable_buffer& buffer) const override {};

    void Deserialize(const boost::asio::mutable_buffer& buffer) override {};

    void Handle(std::shared_ptr<skymarlin::network::Session> session) override {}

    size_t Length() const override { return 0; };

    static constexpr PacketType Type = 0x11;
};

TEST(PacketResolver, Register)
{
    const std::vector<std::pair<PacketType, PacketCreator>> creators = {
        {TestPacket::Type, [] { return std::make_unique<TestPacket>(); }},
    };
    PacketResolver::Init(creators);

    if (const auto test_packet = PacketResolver::Resolve(TestPacket::Type); !test_packet) {
        FAIL();
    }
}
}
