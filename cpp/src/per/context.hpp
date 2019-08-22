#ifndef __PER_CONTEXT_HPP__
#define __PER_CONTEXT_HPP__

#include <string>

#include "utils/signal.hpp"

namespace per {
    class tree;

    /**
     * A context has memory ownership over a particular tree
     * On context destroy, the tree will also be deleted
     */
    class context {
    public:
        context(const std::string& name);
        ~context();

        constexpr const std::string& get_name() const { return name_; }

        constexpr tree* get_tree() { return tree_; }
        constexpr const tree* get_tree() const { return tree_; }

        constexpr void set_tree(tree* tree) { tree_ = tree; }

        signal<> on_dispose;
    private:
        std::string name_;
        tree* tree_;
    };
}

#endif
