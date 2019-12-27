#include "main.hpp"

using namespace telegraph;
using namespace std::string_literals;

void die(const std::string& message) {
    std::cerr << "telegraph_server: " << message << std::endl;
    exit(1);
}

int main(int argc, char** argv) {
    context* live_ctx = nullptr;
    // if (argc < 2) {
    //     die("missing config file path");
    // }

    if (argc == 2) {
        tree* live_tree = read_config_file(argv[1]);
        context* live_ctx = new context("live");
        live_ctx->set_tree(live_tree); // ctx now owns t and will free t on destruction

        std::cout << "live: " << *live_tree << std::endl;
    }

    tree* system_tree = get_system_tree();
    context* system_ctx = new context("system");
    system_ctx->set_tree(system_tree);

    std::cout << "system: " << *system_tree << std::endl;

    server srv;
    srv.start("0.0.0.0:8000");

    {
        auto cl = srv.contexts.lock();
        if (live_ctx != nullptr) {
            srv.contexts.add(live_ctx);
        }
        srv.contexts.add(system_ctx);
    }

    srv.join();
}
