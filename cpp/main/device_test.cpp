#include <telegraph/local/device.hpp>
#include <telegraph/common/nodes.hpp>


#include <iostream>

using namespace telegraph;

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: device_test <port> <baud>" << std::endl;
        return 1;
    }
    const std::string port = argv[1];
    std::vector<int> bauds = {115200};
    const int baud = std::stoi(argv[2]);

    auto dev = std::make_shared<local_device>(port, bauds);
    auto task = dev->create_task("io_task", baud);

    io::io_context ioc;
    io::spawn(ioc,
        [&](io::yield_context yield) {
            // start the io task
            task->start(yield, ioc);
            std::cout << "conntected to port!" << std::endl;

            // fetch the tree
            auto tree = dev->fetch(yield); 
            if (!tree) {
                std::cerr << "unable to fetch tree" << std::endl;
                return;
            }
            std::cout << *tree << std::endl;
        });
    ioc.run();
}
