#include "group.hpp"

#include "../errors.hpp"

#include <algorithm>
#include <iomanip>

namespace per {
    group::group(const std::string& name, const std::string& pretty, const std::string& desc,
                    const std::string& schema, int version) :
                    node(name, pretty, desc), schema_(schema), 
                    version_(version), children_() {}

    void
    group::add_child(const std::shared_ptr<node>& node) {
        if (std::find(children_.begin(), children_.end(), node) != children_.end()) return;
        auto parent = node->get_parent().lock();
        if (parent != nullptr) {
            parent->remove_child(node);
        }
        node->set_parent(std::static_pointer_cast<group>(shared_from_this()));
        children_.push_back(node);
        named_children_[node->get_name()] = node;

        child_added(node);
        // now go through 
        // all of the child nodes and signal a node added
        // in an order such that the parents always get added
        // first
        node->visit([this](const shared_node& n) {
                    child_added(n);
        }, true);
    }

    void
    group::remove_child(const std::shared_ptr<node>& node) {
        auto pos = std::find(children_.begin(), children_.end(), node);
        if (pos == children_.end()) return;
        children_.erase(pos);
        named_children_.erase(node->get_name());
        node->set_parent(std::weak_ptr<group>());

        // now go through all of the child nodes
        // and signal a node removed
        node->visit([this](const shared_node& n) {
            child_removed(n);
        }, false);
    }

    shared_node
    group::operator[](const std::string& name) {
        try {
            return named_children_.at(name);
        } catch (const std::out_of_range& e) {
            return nullptr;
        }
    }

    shared_const_node
    group::operator[](const std::string& name) const {
        try {
            return named_children_.at(name);
        } catch (const std::out_of_range& e) {
            return nullptr;
        }
    }

    void
    group::visit(const std::function<void(const shared_node&)>& visitor, bool preorder) {
        if (preorder) visitor(shared_from_this());
        for (const auto&c : children_) {
            c->visit(visitor, preorder);
        }
        if (!preorder) visitor(shared_from_this());
    }

    void
    group::visit(const std::function<void(const shared_const_node&)>& visitor, bool preorder) const {
        if (preorder) visitor(shared_from_this());
        for (const auto&c : children_) {
            ((const node*) c.get())->visit(visitor, preorder);
        }
        if (!preorder) visitor(shared_from_this());
    }

    void
    group::print(std::ostream& o, int ident) const {
        o << std::setfill(' ') << std::setw(ident) << "";
        o << get_name() << ' ' << get_schema() << ' ' << get_version() << std::endl;
        ident += 4;
        for (auto& n : children_) {
            n->print(o, ident);
        }
    }
}
