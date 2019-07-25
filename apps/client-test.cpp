#include <per/utils/hocon.hpp>
#include <per/utils/dummy_source.hpp>

#include <per/nodes/node.hpp>
#include <per/nodes/variable.hpp>
#include <per/nodes/group.hpp>
#include <per/nodes/tree.hpp>

#include <per/client.hpp>

#include <iostream>

#include <rethinkdb.h>

using namespace per;

int main(int argc, char** argv) {
    hocon_parser parser;
    json j = parser.parse_file("../apps/example.conf");

    tree t = tree::unpack(j["root"]);

    try {
        client c("localhost", 28015);
        c.replace("live", t);
    } catch (RethinkDB::Error& e) {
        std::cout << e.message << std::endl;
    }
}
