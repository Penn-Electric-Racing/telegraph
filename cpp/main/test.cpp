#include <telegraph/utils/hocon.hpp>
#include <telegraph/gen/config.hpp>
#include <telegraph/ns/nodes.hpp>

#include <iostream>
#include <filesystem>

using namespace telegraph;

int main(int argc, char** argv) {
    char* dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");
    std::string file = (dir ? std::string(dir) : ".") + "/cpp/main/example.conf";
    hocon_parser parser;
    json j = parser.parse_file(file);
    // parse the json
    config c(j);
    std::cout << *(c.get_tree()) << std::endl;
}
