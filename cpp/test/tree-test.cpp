#include <telegraph/utils/hocon.hpp>
#include <telegraph/gen/config.hpp>
#include <telegraph/common/nodes.hpp>

#include <iostream>

#include <filesystem>

using namespace telegraph;

int main(int argc, char** argv) {
    hocon_parser parser;
    json j = parser.parse_file("cpp/test/example.conf");
    config c(j);
    const node* t = c.get_tree();
    std::cout << *t << std::endl;
}
