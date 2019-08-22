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
        ~group();

        constexpr const std::string& get_schema() const { return schema_; }
        constexpr int get_version() const { return version_; }

        void add_child(node* node);
        void remove_child(node* node);

        node* operator[](const std::string& name) override;
        const node* operator[](const std::string& name) const override;

        void print(std::ostream& o, int ident) const override;

        signal<node*&> on_child_added;
        signal<node*&> on_child_removed;
    private:
        std::string schema_;
        int version_;

        std::vector<node*> children_; // a group owns the memory to the children
        std::unordered_map<std::string, node*> children_map_;
    };
}

#endif
