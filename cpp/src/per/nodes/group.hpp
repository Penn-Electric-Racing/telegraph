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

        void child_added(const std::shared_ptr<node>& node);
        void child_removed(const std::shared_ptr<node>& node);

        shared_node operator[](const std::string& name) override;
        shared_const_node operator[](const std::string& name) const override;

        void visit(const std::function<void(const shared_node&)>& visitor, 
                        bool preorder=true) override;
        void visit(const std::function<void(const shared_const_node&)>& visitor,
                        bool preorder=true) const override;

        void print(std::ostream& o, int ident) const override;

        // for listening to child add/remove requests
        signal<const shared_node&> on_add_child;
        signal<const shared_node&> on_remove_child;

        // on child added/removed 
        // (if connected to database, a remove person adding a node can trigger this)
        signal<const shared_node&> on_child_added;
        signal<const shared_node&> on_child_removed;
    private:
        std::string schema_;
        int version_;

        std::vector<shared_node> children_;
        std::unordered_map<std::string, shared_node> named_children_;
    };
}

#endif
