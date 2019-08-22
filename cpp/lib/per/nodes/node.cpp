#include "node.hpp"

namespace per {
    node::node(const std::string& name, 
            const std::string& pretty, const std::string& desc) : 
                on_descendant_added(), on_descendant_removed(),
                name_(name), pretty_(pretty), desc_(desc), parent_() {}

    node::~node() {
        on_dispose();
    }

    node*
    node::find(const std::string& path) {
        return nullptr;
    }

    const node*
    node::find(const std::string& path) const {
        return nullptr;
    }

    node*
    node::operator[](const std::string& name) {
        return nullptr;
    }

    const node*
    node::operator[](const std::string& name) const {
        return nullptr;
    }

    std::ostream& operator<<(std::ostream& o, const node& n) {
        n.print(o, 0);
        return o;
    }
}
