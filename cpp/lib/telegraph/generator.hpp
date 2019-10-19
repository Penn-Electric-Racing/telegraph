#ifndef __TELEGRAPH_GENERATOR_HPP__
#define __TELEGRAPH_GENERATOR_HPP__

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
            inline target() : filename(), tree_include(),
                tree_target(nullptr), configs() {}
            std::string filename;

            // either tree_include or tree should be set
            std::string tree_include;
            tree* tree_target;

            // we can have as many configs as we want
            std::vector<config*> configs;
        };
        struct result {
            inline result() : filename(), code() {}
            std::string filename;
            std::string code;
        };

        generator();

        // add targets
        void set_tree(const std::string& filename, 
                            tree* t);
        void set_tree_include(const std::string& filename, 
                                const std::string& tree_include);
        void add_config(const std::string& filename, 
                            config* conf);

        inline void set_namespace(const std::string& ns) { namespace_ = ns; }

        // fills in the targets
        std::vector<result> generate(const id_map& ids) const;

        inline void set_output_dir(const std::string& d) { output_dir_ = d; }

        // does the generation and writes the files
        void run(const id_map& ids) const;
    private:
        std::string generate_types(const tree* t) const;

        std::string generate_node(const node* n, const id_map& ids,
                                    const std::string& accessor_prefix,
                                    std::map<int32_t, std::string>* id_accessors) const;

        std::string generate_tree(const tree* t, const id_map& ids) const;

        std::string generate_config(const config* c, 
                                    const tree* t, const id_map& ids) const;

        result generate_target(const target& t, const id_map& ids) const;

        // generator config
        std::string namespace_;
        std::unordered_map<std::string, target> targets_;

        // output config
        std::string output_dir_;
    };
}

#endif
