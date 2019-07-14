#include "node.hpp"

namespace per {
    node::node() : name_(), pretty_(), desc_() {}

    node::node(const std::string& name) : name_(name), pretty_(), desc_() {}
}
