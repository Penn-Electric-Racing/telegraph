#include <per/utils/hocon.hpp>
#include <per/utils/dummy_source.hpp>

#include <per/nodes/node.hpp>
#include <per/nodes/variable.hpp>
#include <per/nodes/group.hpp>
#include <per/nodes/tree.hpp>

#include <iostream>

using namespace per;

int main(int argc, char** argv) {
    hocon_parser parser;
    json j = parser.parse_file("../apps/example.conf");

    tree t = tree::unpack(j["root"]);
    std::cout << t << std::endl;
    shared_node n = t.find("foo");
    shared_variable v = std::dynamic_pointer_cast<variable>(n);
    if (v) {
        std::cout << *v << std::endl;
        dummy_source s;
        v->set_source(&s);
        // subscribe
        variable::subscription* sub = v->subscribe();
        if (sub) {
            sub->notify([](const value& v) {
                std::cout << "got: " << v << std::endl;
            });
        }
        for (uint8_t i = 0; i < 30; i++) {
            s.update(v.get(), i);
        }
    }
}
