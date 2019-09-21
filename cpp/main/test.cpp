#include <telegraph/utils/hocon.hpp>

#include <telegraph/context.hpp>
#include <telegraph/server.hpp>

#include <telegraph/nodes/node.hpp>
#include <telegraph/nodes/variable.hpp>
#include <telegraph/nodes/group.hpp>
#include <telegraph/nodes/tree.hpp>

#include <iostream>

#include <filesystem>

using namespace telegraph;


int main(int argc, char** argv) {
    char* dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");
    std::string file = (dir ? std::string(dir) : ".") + "/cpp/main/example.conf";
    hocon_parser parser;
    json j = parser.parse_file(file);

    tree* t = tree::unpack(j["root"]);
    std::cout << *t << std::endl;

    context* ctx = new context("live");
    ctx->set_tree(t); // ctx now owns t and will free t on destruction

    server srv;
    srv.start("0.0.0.0:8000");

    {
        auto cl = srv.contexts.lock();
        srv.contexts.add(ctx);
    }

    srv.join();
}
