#include "group.hpp"

#include "../errors.hpp"

#include <algorithm>

namespace per {

    group::group() : node(), schema_(), version_(0), children_() {}

    group::group(const std::string& name, const std::string& schema, int version) :
                    node(name), schema_(schema), version_(version), children_() {}

    void
    group::add_child(const std::shared_ptr<node>& node) {
        if (std::find(children_.begin(), children_.end(), node) != children_.end()) return;

        children_.push_back(node);
        on_child_added(node);
    }

    void
    group::remove_child(const std::shared_ptr<node>& node) {
        auto pos = std::find(children_.begin(), children_.end(), node);
        if (pos == children_.end()) return;
        children_.erase(pos);
        on_child_removed(node);
    }
}
