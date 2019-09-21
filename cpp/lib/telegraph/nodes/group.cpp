#include "group.hpp"

#include "../errors.hpp"

#include <algorithm>
#include <iomanip>

namespace telegraph {
    group::group(const std::string& name, const std::string& pretty, const std::string& desc,
                    const std::string& schema, int version) :
                    node(name, pretty, desc), 
                    on_child_added(), on_child_removed(),
                    schema_(schema), 
                    version_(version), children_(), children_map_() {}

    group::~group() {
        for (node* n : children_) {
            delete n;
        }
    }

    void
    group::add_child(node* n) {
        //if (node == nullptr)
        //if (children_map_[node->get_name()] != nullptr)
        if (std::find(children_.begin(), children_.end(), n) != children_.end()) return;
        auto parent = n->get_parent();
        if (parent != nullptr) {
            parent->remove_child(n);
        }

        n->set_parent(this);
        idx_map_[n] = children_.size();
        children_.push_back(n);
        children_map_[n->get_name()] = n;

        on_child_added(n);
        // go through all of the children
        // of this node and call on_descendant_added
        on_descendant_added(n);

        // register the function to *this* object
        // so we can easily remove it later
        n->on_descendant_added.add(this, 
                [this](node* d) {on_descendant_added(d);});
        n->on_descendant_removed.add(this, 
                [this](node* d) {on_descendant_removed(d);});
    }

    void
    group::remove_child(node* n) {
        auto pos = std::find(children_.begin(), children_.end(), n);
        if (pos == children_.end()) return;
        auto next = pos;
        while (next != children_.end()) {
            next++;
            node* n = (*pos);
            idx_map_[n] = idx_map_[n]--;
        }
        children_.erase(pos);
        children_map_.erase(n->get_name());
        n->set_parent(nullptr);

        n->on_descendant_added.remove(this);
        n->on_descendant_removed.remove(this);

        on_child_removed(n);
    }

    node* 
    group::operator[](const std::string& name) {
        try {
            return children_map_.at(name);
        } catch (const std::out_of_range& e) {
            return nullptr;
        }
    }

    const node*
    group::operator[](const std::string& name) const {
        try {
            return children_map_.at(name);
        } catch (const std::out_of_range& e) {
            return nullptr;
        }
    }

    void
    group::print(std::ostream& o, int ident) const {
        o << std::setfill(' ') << std::setw(ident) << "";
        o << get_name() << ' ' << get_schema() << ' ' << get_version();
        ident += 4;
        for (auto& n : children_) {
            o << std::endl;
            n->print(o, ident);
        }
    }
    void
    group::add_descendants(std::vector<node*>* nodes,
                       bool include_this, bool postorder) {
        if (include_this && !postorder) nodes->push_back(this);
        for (auto& n : children_) {
            n->add_descendants(nodes, true, postorder);
        }
        if (include_this && postorder) nodes->push_back(this);
    }

    void
    group::add_descendants(std::vector<const node*>* nodes,
                        bool include_this, bool postorder) const {
        if (include_this && !postorder) nodes->push_back(this);
        for (const node* n : children_) {
            n->add_descendants(nodes, true, postorder);
        }
        if (include_this && postorder) nodes->push_back(this);
    }
}
