#include "node.hpp"

namespace per {
    node::node() : name_(), pretty_(), desc_() {}

    node::node(const std::string& name) : name_(name), pretty_(), desc_() {}

    std::ostream& operator<<(std::ostream& o, const node& n) {
        n.print(o, 0);
        return o;
    }
}
