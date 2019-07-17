#ifndef __PER_NODES_TREE__
#define __PER_NODES_TREE__

#include <memory>
#include <ostream>

#include "../utils/json_fwd.hpp"

namespace per {
    class group;

    class tree {
    public:
        tree();
        tree(const std::shared_ptr<group>& root);

        const std::shared_ptr<group>& root() { return root_; }
        const std::shared_ptr<const group> root() const { return root_; }

        static tree unpack(const json& j);
    private:
        std::shared_ptr<group> root_;
    };

    std::ostream& operator<<(std::ostream& o, const tree& t);
}
#endif
