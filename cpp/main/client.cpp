#include <telegraph/local/namespace.hpp>
#include <telegraph/remote/client.hpp>
#include <telegraph/common/nodes.hpp>
#include <telegraph/utils/errors.hpp>

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

    std::shared_ptr<local_namespace> ns = std::make_shared<local_namespace>(ioc);
    client conn(ioc, address, port, ns);

    io::spawn(ioc,
        [&](io::yield_context byield) {
            try {
                io::yield_ctx yield(byield);

                // connect and get the rns
                auto rns = conn.connect(yield);
                // now we have the remote namespace

                auto contexts = rns->contexts(yield);
                if (!contexts) {
                    std::cerr << "unable to fetch contexts" << std::endl;
                } else {
                    // iterate through the contexts
                    // we retrieved
                    for (auto i : *contexts) {
                        // fetch the tree
                        auto ctx = i.second;
                        std::cout << "got context: " << ctx->get_name() << std::endl;
                        auto tree = ctx->fetch(yield);
                        if (tree) {
                            std::cout << *tree << std::endl;
                        }
                    }
                }
            } catch (error& e) {
                std::cerr << e.what() << std::endl;
            }
        });
    ioc.run();
}
