#ifndef __PER_NODES_TREE__
#define __PER_NODES_TREE__

#include <memory>
#include <ostream>

#include "../utils/json_fwd.hpp"

#include "node.hpp"

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

        signal<> on_dispose;
    private:
        group* root_;
    };

    std::ostream& operator<<(std::ostream& o, const tree& t);
}
#endif
