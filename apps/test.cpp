#include <per/utils/hocon.hpp>

#include <per/nodes/node.hpp>
#include <per/nodes/group.hpp>
#include <per/nodes/tree.hpp>

#include <iostream>

using namespace per;

int main(int argc, char** argv) {
    hocon_parser parser;
    json j = parser.parse_file("../apps/example.conf");

    tree t = tree::unpack(j["root"]);
    std::cout << t << std::endl;
}
