#include "group.hpp"

#include "../errors.hpp"

#include <algorithm>
#include <iomanip>

namespace per {

    group::group() : node(), schema_(), version_(0), children_() {}

    group::group(const std::string& name, const std::string& schema, int version) :
                    node(name), schema_(schema), version_(version), children_() {}

    void
    group::add_child(const std::shared_ptr<node>& node) {
        if (std::find(children_.begin(), children_.end(), node) != children_.end()) return;

        children_.push_back(node);
    }

    void
    group::remove_child(const std::shared_ptr<node>& node) {
        auto pos = std::find(children_.begin(), children_.end(), node);
        if (pos == children_.end()) return;
        children_.erase(pos);
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
