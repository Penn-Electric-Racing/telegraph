#ifndef __PER_NODES_TREE__
#define __PER_NODES_TREE__

#include "../utils/json_fwd.hpp"

#include "node.hpp"

#include <vector>
#include <memory>
#include <ostream>


namespace per {
    class tree {
    public:
        tree();
        tree(group* root);
        ~tree();

        group* get_root() { return root_; }
        const group* get_root() const { return root_; }

        // will return a new tree, allocated on the heap
        // that you have to free yourself
        static tree *unpack(const json& j);

        std::vector<node*> nodes(bool postorder = false);
        std::vector<const node*> nodes(bool postorder = false) const;

        signal<> on_dispose;

        signal<node*> on_descendant_added;
        signal<node*> on_descendant_removed;
    private:
        group* root_;
    };

    std::ostream& operator<<(std::ostream& o, const tree& t);
}
#endif
