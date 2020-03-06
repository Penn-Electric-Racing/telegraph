#include <telegraph/local/namespace.hpp>
#include <telegraph/remote/relay.hpp>
#include <telegraph/common/nodes.hpp>

#include <iostream>
#include <filesystem>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/spawn.hpp>

using namespace telegraph;

int main(int argc, char** argv) {
    auto const address = "127.0.0.1";
    auto const port = "8081";

    boost::asio::io_context ioc;
    local_namespace ns;
    relay client(ioc, &ns);
    auto conn = client.connect(address, port);
    io::spawn(ioc,
        [&](io::yield_context yield) {
            io::yield_ctx c(yield);
            io::executor_work_guard<io::io_context::executor_type> work(ioc.get_executor());

            // do the actual connection
            auto rns = conn->connect(c);
            // now we have the remote namespace

            auto contexts = rns->contexts(c);
            if (!contexts) {
                std::cerr << "unable to fetch contexts" << std::endl;
            } else {
                // iterate through the contexts
                // we retrieved
                for (auto i : *contexts) {
                    // fetch the tree
                    auto ctx = i.second;
                    std::cout << "got context: " << ctx->get_name() << std::endl;
                    auto tree = ctx->fetch(c);
                    if (tree) {
                        std::cout << *tree << std::endl;
                    }
                }
            }
        });
    ioc.run();
}
