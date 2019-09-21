#include "generator.hpp"

#include "config.hpp"

#include "nodes/node.hpp"
#include "nodes/group.hpp"
#include "nodes/action.hpp"
#include "nodes/variable.hpp"
#include "nodes/stream.hpp"

#include "nodes/tree.hpp"

#include "id_map.hpp"

#include "errors.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace telegraph {

    generator::generator() : namespace_(), targets_(),
                             output_dir_() {}

    void
    generator::set_tree(const std::string& filename,
                            tree* t) {
        targets_[filename].filename = filename;
        targets_[filename].tree_include.clear();
        targets_[filename].tree_target = t;
    }


    void
    generator::set_tree_include(const std::string& filename,
                            const std::string& tree_include) {
        targets_[filename].filename = filename;
        targets_[filename].tree_include = tree_include;
        targets_[filename].tree_target = nullptr;
    }

    void
    generator::add_config(const std::string& filename,
                            config* conf) {
        targets_[filename].filename = filename;
        targets_[filename].configs.push_back(conf);
    }

    // helper indent function
    static void indent(std::string& s, int spaces) {
        if (s.length() == 0) return;
        s.insert(0, spaces, ' ');
        size_t pos = s.find("\n");
        while (pos != std::string::npos) {

            // insert n spaces after \n
            s.insert(pos + 1, spaces, ' ');

            // find the next \n
            pos = s.find("\n", pos + 1);
        }
    }

    std::string
    generator::generate_node(const node* n, const id_map& ids) const {
        int32_t id = ids[n];
        std::string code;
        if (dynamic_cast<const group*>(n) != nullptr) {
            const group* g = dynamic_cast<const group*>(n);
            code += "struct {\n";
            for (const node* c : *g) {
                std::string subcode = generate_node(c, ids);
                indent(subcode, 4);
                code += subcode;
                code += "\n";
            }
            code += "} " + g->get_name() + ";";
        } else if (dynamic_cast<const variable*>(n) != nullptr) {
            const variable* v = dynamic_cast<const variable*>(n);
            code += "constexpr variable<> " + v->get_name() + 
                        " = variable<>(" + std::to_string(id) + ");";
        } else if (dynamic_cast<const action*>(n) != nullptr) {
            const action* a = dynamic_cast<const action*>(n);
            code += "constexpr action<> " + a->get_name() + 
                        " = action<>(" + std::to_string(id) + ");";
        } else if (dynamic_cast<const stream*>(n) != nullptr) {
            code += "stream not yet implemented!!!";
        }
        return code;
    }

    std::string
    generator::generate_tree(const tree* t, const id_map& ids) const {
        std::string code = "struct node_tree {\n";
        code += "    constexpr const int version = " + std::to_string(t->get_root()->get_version()) + ";\n";
        for (const node* n : *t->get_root()) {
            std::string subcode = generate_node(n, ids);
            indent(subcode, 4);
            code += subcode;
            code += "\n";
        }
        code += "};";
        return code;
    }

    std::string
    generator::generate_config(const config* c, const tree *t, const id_map& ids) const {
        std::string code =;
            "struct " + c->get_name() + "_config {\n";

        for (const std::string& set_name : c->sets()) {
            const std::unordered_set<node*>& set = c->get(set_name);
            // now generate the std::array
        }
        code += "};"
        return code;
    }

    generator::result
    generator::generate_target(const generator::target& t, const id_map& ids) const {
        std::string code =
            "#pragma once\n\n"
            "#include <telegraph/gen/variable.hpp>\n"
            "#include <telegraph/gen/action.hpp>\n"
            "#include <telegraph/gen/stream.hpp>\n\n";

        // now include the tree file we if want to do that
        if (t.tree_include.length() > 0) {
            code += "#include \"" + t.tree_include + "\"\n\n";
        }

        // add the namespace wrapper
        if (namespace_.length() > 0) {
            code += "namespace " + namespace_ + " {\n\n";
        }

        // now generate the tree code if there is any
        if (t.tree_target) {
            std::string subcode = generate_tree(t.tree_target, ids);
            if (namespace_.length() > 0) indent(subcode, 4);
            code += subcode;
            code += "\n\n";
        }

        tree* tr = nullptr;
        try {
            tr = t.tree_target ? t.tree_target :
                        targets_.at(t.tree_include).tree_target;
        } catch (std::out_of_range& e) {}

        if (!tr) throw missing_error("Could not find tree");

        for (config* c : t.configs) {
            std::string subcode = generate_config(c, tr, ids);
            if (namespace_.length() > 0) indent(subcode, 4);
            code += subcode;
            code += "\n\n";
        }

        // end the namespace wrapper
        if (namespace_.length() > 0) {
            code += "\n}";
        }

        result r;
        r.filename = t.filename;
        r.code = std::move(code);
        return r;
    }

    std::vector<generator::result>
    generator::generate(const id_map& ids) const {
        std::vector<result> r;
        for (const auto& i : targets_) {
            r.push_back(generate_target(i.second, ids));
        }
        return r;
    }


    void
    generator::run(const id_map& ids) const {
        std::vector<generator::result> results = generate(ids);
        for (const generator::result& r : results) {
            std::filesystem::path p = std::filesystem::path(output_dir_.length() > 0 ? output_dir_ : "./") / r.filename;
            std::ofstream out(p);
            out << r.code << std::flush;
            out.close();
        }
    }
}
