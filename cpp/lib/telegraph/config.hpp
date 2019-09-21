#ifndef __TELEGRAPH_CONFIGURATION_HPP__
#define __TELEGRAPH_CONFIGURATION_HPP__

#include "utils/json_fwd.hpp"
#include "nodes/node.hpp"

#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace telegraph {
    class tree;

    // configs are named sets of nodes
    class config {
    public:
        config(const std::string& name);
        config(std::string&& name);

        constexpr const std::string& get_name() const { return name_; }

        std::unordered_set<std::string> sets() const;

        std::unordered_set<node*> &get(const std::string& name);
        const std::unordered_set<node*> &get(const std::string& name) const;

        void add(const std::string& name, const std::unordered_set<node*>& nodes);
        void add(const std::string& name, std::unordered_set<node*>&& nodes);

        // the returned configs are allocated on the heap
        static std::vector<config*> unpack(tree* t, const json& config);
    private:
        std::string name_;
        std::unordered_map<std::string, std::unordered_set<node*>> sets_;
    };
}
#endif
