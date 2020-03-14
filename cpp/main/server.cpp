#include <telegraph/local/namespace.hpp>
#include <telegraph/local/device.hpp>
#include <telegraph/remote/server.hpp>

#include <iostream>
#include <filesystem>

#include <boost/asio/io_context.hpp>

using namespace telegraph;

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

int main(int argc, char** argv) {
    std::cout << "starting server..." << std::endl;

    boost::asio::io_context ctx;

    std::shared_ptr<local_namespace> ns = std::make_shared<local_namespace>(ctx);
    ns->register_task_factory("device_scanner",
        [] () {
    });

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
