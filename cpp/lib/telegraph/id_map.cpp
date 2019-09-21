#include "id_map.hpp"

#include "nodes/group.hpp"
#include "nodes/node.hpp"
#include "nodes/tree.hpp"

namespace telegraph {
    id_map::id_map() : node_id_map_(), id_node_map_(), next_id_(1) {}

    void
    id_map::add(node* n) {
        if (node_id_map_.find(n) != node_id_map_.end()) return;
        int32_t id = next_id_++;
        node_id_map_[n] = id;
        id_node_map_[id] = n;
    }

    void
    id_map::add_leaves(tree* t) {
        std::vector<node*> nodes = t->nodes();
        for (node* n : nodes) {
            if (dynamic_cast<group*>(n) != nullptr) continue;
            add(n);
        }
    }
}
