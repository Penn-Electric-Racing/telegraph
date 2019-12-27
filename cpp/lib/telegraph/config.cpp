#include "config.hpp"

#include "errors.hpp"

#include "utils/json.hpp"

#include "tree.hpp"
#include "nodes/node.hpp"
#include "nodes/group.hpp"
#include "nodes/variable.hpp"

#include <sstream>
#include <iostream>

namespace telegraph {
    config::config(const std::string& name) : name_(name), sets_() {}
    config::config(std::string&& name) : name_(name) {}

    std::unordered_set<std::string>
    config::sets() const {
        std::unordered_set<std::string> s;
        for (auto a : sets_) {
            s.insert(a.first);
        }
        return s;
    }

    std::unordered_set<node*>&
    config::get(const std::string& name) {
        return sets_.at(name);
    }

    const std::unordered_set<node*>&
    config::get(const std::string& name) const {
        return sets_.at(name);
    }

    void
    config::add(const std::string& name, const std::unordered_set<node*>& nodes) {
        std::unordered_set<node*> s;
        for (node* n : nodes) {
            s.insert(n);
        }
        sets_[name] = std::move(s);
    }

    void
    config::add(const std::string& name, std::unordered_set<node*>&& nodes) {
        sets_[name] = std::move(nodes);
    }

    static void populate_config(config* conf, 
                                const std::vector<node*>& nodes, 
                                const json& j) {
        for (const auto& item : j.items()) {
            const json& filter = item.value();

            const std::string& name = item.key();
            std::vector<std::string> includes =
                filter.value<std::vector<std::string>>("includes",{});
            std::vector<std::string> excludes =
                filter.value<std::vector<std::string>>("excludes",{});

            std::unordered_set<node*> set;
            for (node* n : nodes) {
                std::string path = n->get_path();
                bool inc = false;
                for (const std::string& s : includes) {
                    if (path.size() > s.size() &&
                            path.substr(0, s.size()) == s) {
                        inc = true;
                        break;
                    }
                }
                if (inc) {
                    for (const std::string& s : excludes) {
                        if (path.size() > s.size() &&
                                path.substr(0, s.size()) == s) {
                            inc = false;
                            break;
                        }
                    }
                }
                if (inc) set.insert(n);
            }
            conf->add(name, std::move(set));
        }
    }

    std::vector<config*>
    config::unpack(tree* t, const json& configs) {
        std::vector<node*> nodes = t->nodes();
        std::vector<config*> confs;
        for (const auto& item : configs.items()) {
            const std::string& name = item.key();
            config* c = new config(std::move(name));
            populate_config(c, nodes, item.value());
            confs.push_back(c);
        }
        return confs;
    }
}
