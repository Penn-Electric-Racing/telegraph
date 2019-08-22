#include <per/utils/hocon.hpp>

#include <per/nodes/node.hpp>
#include <per/nodes/variable.hpp>
#include <per/nodes/group.hpp>
#include <per/nodes/tree.hpp>

#include <iostream>

#include <filesystem>

using namespace per;

int main(int argc, char** argv) {
    char* dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");
    std::string file = (dir ? std::string(dir) : ".") + "/cpp/main/example.conf";
    hocon_parser parser;
    json j = parser.parse_file(file);

    tree* t = tree::unpack(j["root"]);
    std::cout << *t << std::endl;
}
