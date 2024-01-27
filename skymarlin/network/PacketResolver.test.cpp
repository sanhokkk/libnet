#include <gtest/gtest.h>
#include <skymarlin/network/PacketResolver.hpp>
#include <skymarlin/network/Packet.hpp>

namespace skymarlin::network {
class TestPacket : public Packet {
public:
    void Serialize(boost::asio::mutable_buffer& buffer) const override {};

    void Deserialize(const boost::asio::mutable_buffer& buffer) override {};

    void Handle(std::shared_ptr<skymarlin::network::Session> session) override {}

    size_t Length() const override { return 0; };

    static constexpr PacketType Type = 0x11;
};

TEST(PacketResolver, Register) {
    std::vector<std::pair<PacketType, PacketCreator>> creators = {
        {TestPacket::Type, [] { return std::make_unique<TestPacket>(); }}
    };
    PacketResolver::Init(creators);

    std::unique_ptr<TestPacket> test_packet;
    if (!PacketResolver::TryResolve(TestPacket::Type, test_packet)) {
        FAIL();
    }
}
}


/*int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}*/
