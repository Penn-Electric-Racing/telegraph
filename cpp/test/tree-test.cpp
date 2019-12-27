#include <telegraph/utils/hocon.hpp>

#include <telegraph/context.hpp>
#include <telegraph/server.hpp>

#include <telegraph/nodes/node.hpp>
#include <telegraph/nodes/variable.hpp>
#include <telegraph/nodes/group.hpp>
#include <telegraph/tree.hpp>

#include <iostream>

#include <filesystem>

using namespace telegraph;


int main(int argc, char** argv) {
    hocon_parser parser;
    json j = parser.parse_file("cpp/test/example.conf");
    tree* t = tree::unpack(j["root"]);
    std::cout << *t << std::endl;
}
