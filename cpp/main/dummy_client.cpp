#include <telegraph/utils/hocon.hpp>
#include <telegraph/gen/config.hpp>
#include <telegraph/common/nodes.hpp>

#include <iostream>
#include <filesystem>

using namespace telegraph;

int main(int argc, char** argv) {
    auto const address = net::ip::make_address("127.0.0.1");
    const unsigned short port = 8081;

    local_namespace ns;
    relay client(&ns);
    client.connect(tcp::endpoint{address, port}, ctx)->run();
    // client is connected
}
