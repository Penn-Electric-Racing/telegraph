#include <telegraph/local/device.hpp>
#include <telegraph/common/nodes.hpp>

#include <telegraph/utils/io.hpp>

#include <iostream>

using namespace telegraph;

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: device_test <port> <baud>" << std::endl;
        return 1;
    }
    const std::string port = argv[1];
    const int baud = std::stoi(argv[2]);

    local_namespace ns;

    io::io_context ioc;

    auto dev_task = std::make_shared<device_io_task>(ioc, "device", port);

    io::spawn(ioc,
        [&](io::yield_context yield) {
            io::yield_ctx c(yield);

            // start the io task
            dev_task->start(c, info(baud));
            auto q = ns.contexts(c, uuid(), std::string(), "device");
            auto dev = q->result();

            // fetch the tree
            auto tree = dev->fetch(c);
            if (!tree) {
                std::cerr << "unable to fetch tree" << std::endl;
                return;
            }
            std::cout << *tree << std::endl;
        });
    ioc.run();
}
