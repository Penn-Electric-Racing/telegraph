#include "node.hpp"

#include "group.hpp"

namespace telegraph {
    node::node(const std::string& name, 
            const std::string& pretty, const std::string& desc) : 
                on_descendant_added(), on_descendant_removed(),
                name_(name), pretty_(pretty), desc_(desc), parent_() {}

    node::~node() {
        on_dispose();
    }

    std::string
    node::get_path() const {
        bool g = dynamic_cast<const group*>(this) != nullptr;
        return (parent_ ? parent_->get_path() + "/" + name_ : "")
                + (g ? "/" : "");
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

    void
    node::add_descendants(std::vector<node*>* n, bool include_this, bool postorder) {
        if (include_this) n->push_back(this);
    }

    void
    node::add_descendants(std::vector<const node*>* n, bool include_this, bool postorder) const {
        if (include_this) n->push_back(this);
    }

    std::ostream& operator<<(std::ostream& o, const node& n) {
        n.print(o, 0);
        return o;
    }
}
