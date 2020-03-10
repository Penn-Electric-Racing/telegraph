#include <telegraph/utils/hocon.hpp>
#include <telegraph/gen/config.hpp>
#include <telegraph/common/nodes.hpp>
#include <telegraph/local/namespace.hpp>
#include <telegraph/remote/server.hpp>

#include <iostream>
#include <filesystem>

#include <boost/asio/io_context.hpp>

using namespace telegraph;

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

int main(int argc, char** argv) {
    std::cout << "starting dummy server..." << std::endl;

    char* dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");
    std::string file = (dir ? std::string(dir) : ".") + "/cpp/main/example.conf";
    hocon_parser parser;
    json j = parser.parse_file(file);
    // parse the json
    config c(j);
    const node* t = c.get_tree();

    std::cout << "read tree: " << std::endl;
    std::cout << *t << std::endl;

    std::shared_ptr<local_namespace> ns = std::make_shared<local_namespace>();

    boost::asio::io_context ctx;

    // start a server on the relay
    // this will enqueue callbacks on the io context
    auto const address = net::ip::make_address("0.0.0.0");
    const unsigned short port = 8081;

    io::spawn(ctx,
        [&](io::yield_context yield) {
            io::yield_ctx c(yield);

            server s(ctx, tcp::endpoint{address,port}, ns);
            s.run(c);
        });

    // process requests on the io context
    ctx.run();
}
