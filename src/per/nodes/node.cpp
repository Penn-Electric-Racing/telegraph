#include "node.hpp"

namespace per {
    node::node(const std::string& name, 
            const std::string& pretty, const std::string& desc) : 
                name_(name), pretty_(pretty), desc_(desc), parent_() {}

    shared_node
    node::find(const std::string& path) {
        return nullptr;
    }

    shared_const_node
    node::find(const std::string& path) const {
        return nullptr;
    }

    shared_node
    node::operator[](const std::string& name) {
        return nullptr;
    }

    shared_const_node
    node::operator[](const std::string& name) const {
        return nullptr;
    }

    void
    node::visit(const std::function<void(const shared_node&)>& visitor, bool preorder) {
        visitor(shared_from_this());
    }

    void
    node::visit(const std::function<void(const shared_const_node&)>& visitor, bool preorder) const {
        visitor(shared_from_this());
    }

    std::ostream& operator<<(std::ostream& o, const node& n) {
        n.print(o, 0);
        return o;
    }
}
