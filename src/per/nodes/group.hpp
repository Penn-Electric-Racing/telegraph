#ifndef __PER_GROUP_HPP__
#define __PER_GROUP_HPP__

#include "node.hpp"
#include "../utils/signal.hpp"

#include <memory>
#include <vector>

namespace per {
    class group : public node {
    public:
        group(const std::string& name, const std::string& pretty, const std::string& desc,
                const std::string& schema, int version);

        constexpr const std::string& get_schema() const { return schema_; }
        constexpr int get_version() const { return version_; }

        void add_child(const std::shared_ptr<node>& node);
        void remove_child(const std::shared_ptr<node>& node);

        shared_node operator[](const std::string& name) override;
        shared_const_node operator[](const std::string& name) const override;

        void visit(const std::function<void(const shared_node&)>& visitor, bool preorder=true) override;
        void visit(const std::function<void(const shared_const_node&)>& visitor, bool preorder=true) const override;
        void print(std::ostream& o, int ident) const override;

        signal<const shared_node&> child_added;
        signal<const shared_node&> child_removed;
    private:
        // for propagating events up the node hierarchy
        void notify_node_added(const shared_node& n);
        void notify_node_remvoed(const shared_node& n);

        std::string schema_;
        int version_;

        std::vector<shared_node> children_;
        std::unordered_map<std::string, shared_node> named_children_;
    };
}

#endif
