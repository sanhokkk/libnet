#pragma once

#include <skymarlin/network/Listener.hpp>

namespace skymarlin::network {
class Server : boost::noncopyable {
public:
    Server();

    void Run();

protected:
};
}
