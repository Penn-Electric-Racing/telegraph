#ifndef __TELEGRAPH_GENERATOR_HPP__
#define __TELEGRAPH_GENERATOR_HPP__

#include "config.hpp"
#include "../common/nodes.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <map>

namespace telegraph {
    class node;
    class tree;
    class config;
    class id_map;

    class generator {
    public:
        // a target is a container
        struct target {
            inline target() : filename(), name_space(), 
                tree_include(), tree_target(nullptr), profiles() {}
            std::string filename;
            std::string name_space;

            // either tree_include or tree should be set
            std::string tree_include;
            const node* tree_target;

            // we can have as many configs as we want
            std::vector<const profile*> profiles;
        };
        struct result {
            inline result() : filename(), code() {}
            std::string filename;
            std::string code;
        };

        generator();

        // add targets
        void set_tree(const std::string& filename, const node* t);
        void set_tree_include(const std::string& filename, const std::string& tree_include);
        void set_namespace(const std::string& filename, const std::string& ns);
        void add_profile(const std::string& filename, const profile* conf);

        // fills in the targets
        std::vector<result> generate() const;
    private:
        std::string generate_types(const node* root) const;
        std::string generate_node(const node* n, const std::string& accessor_prefix,
                                    std::map<int32_t, std::string>* id_accessors, 
                                    bool root) const;

        std::string generate_tree(const node* t) const;
        std::string generate_profile(const profile* p) const;
        result generate_target(const target& t) const;

        // generator config
        std::string namespace_;
        std::unordered_map<std::string, target> targets_;
    };
}

#endif
