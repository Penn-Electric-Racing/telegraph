#include "generator.hpp"

#include "config.hpp"

#include "../common/nodes.hpp"
#include "../utils/errors.hpp"

#include <fstream>
#include <iostream>

namespace telegraph {
    generator::generator() : namespace_(), targets_() {}

    void
    generator::set_tree(const std::string& filename, const node* t) {
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
    generator::set_namespace(const std::string& filename, const std::string& ns) {
        targets_[filename].filename = filename;
        targets_[filename].name_space = ns;
    }

    void
    generator::add_profile(const std::string& filename,
                            const profile* p) {
        targets_[filename].filename = filename;
        targets_[filename].profiles.push_back(p);
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

    static std::string type_to_cpp_builtin(const type& t) {
        switch(t.get_class()) {
            case type::None:    return "none";
            case type::Bool:    return "bool";
            case type::Uint8:   return "uint8_t";
            case type::Uint16:  return "uint16_t";
            case type::Uint32:  return "uint32_t";
            case type::Uint64:  return "uint64_t";
            case type::Int8:    return "int8_t";
            case type::Int16:   return "int16_t";
            case type::Int32:   return "int32_t";
            case type::Int64:   return "int64_t";
            case type::Float:   return "float";
            case type::Double:  return "double";
            default: throw missing_error("Not a builtin, must have name!");
        }
    }

    // returns an identifier for a type
    static std::string type_to_cpp_ident(const type& t) {
        if (t.get_name().length() > 0) {
            // return the c++ ified type name
            return t.get_name();
        } else {
            return type_to_cpp_builtin(t);
        }
    }

    static std::string type_to_name(const type& t) {
        if (t.get_name().length() > 0) return t.get_name();
        switch(t.get_class()) {
            case type::None:    return "none";
            case type::Bool:    return "bool";
            case type::Uint8:   return "uint8";
            case type::Uint16:  return "uint16";
            case type::Uint32:  return "uint32";
            case type::Uint64:  return "uint64";
            case type::Int8:    return "int8";
            case type::Int16:   return "int16";
            case type::Int32:   return "int32";
            case type::Int64:   return "int64";
            case type::Float:   return "float";
            case type::Double:  return "double";
            default: throw missing_error("Not a builtin, must have name!");
        }
    }

    static std::string enum_label_to_cpp(const std::string& label) {
        std::string ret = label;
        std::for_each(ret.begin(), ret.end(), [](char& c){
                    c = toupper(c);
                    if (c == ' ') c = '_';
                });
        return ret;
    }

    std::string
    generator::generate_types(const node* tree) const {
        // all the type names we need to generate
        std::map<std::string, const type*> types;
        for (const node* n : tree->nodes()) {
            // if n is a variable or an action add the types to a generation list
            if (dynamic_cast<const action*>(n) != nullptr) {
                const action* a = dynamic_cast<const action*>(n);
                const type& arg = a->get_arg_type();
                const type& ret = a->get_ret_type();
                if (arg.get_name().length() > 0) types[arg.get_name()] = &arg;
                if (ret.get_name().length() > 0) types[ret.get_name()] = &ret;
            } else if (dynamic_cast<const variable*>(n) != nullptr) {
                const variable* v = dynamic_cast<const variable*>(n);
                const type& t = v->get_type();
                if (t.get_name().length() > 0) types[t.get_name()] = &t;
            }
        }

        std::string code;
        // go through each of the types
        for (auto elem : types) {
            const type* tp = elem.second;
            if (tp->get_class() == type::Enum) {
                // generate an enum
                code += "enum class " + tp->get_name() + " : uint8_t {\n";
                std::string subcode = "";
                // go through each of the labels and convert to cpp style, joining by commas
                bool first = true;
                for (const std::string& l : tp->get_labels()) {
                    if (first) first = false;
                    else subcode += ",\n";
                    subcode += enum_label_to_cpp(l);
                }
                indent(subcode, 4);
                subcode += "\n";
                code += subcode;
                code += "};";
            } else {
                // put in a typedef!
                code += "typedef " + type_to_cpp_builtin(*tp) + " " + tp->get_name() + ";";
            }
            code += "\n";

            code += "constexpr const char* " + type_to_name(*tp) + "_labels_[] = {";
            bool first = true;
            for (const std::string& l : tp->get_labels()) {
                if (first) first = false;
                else code += ",";
                code += "\"" + l + "\"";
            }
            code += "};\n";

            code += "telegen::type_info<" + type_to_cpp_ident(*tp) + "> " + type_to_name(*tp) + "_type " +
                        "= telegen::type_info<" + type_to_cpp_ident(*tp) + ">(\"" +  
                        tp->get_name() + "\", " + std::to_string(tp->get_labels().size()) + ", " + type_to_name(*tp) + "_labels_);";

            code += "\n\n";
        }

        return code;
    }

    std::string
    generator::generate_node(const node* n,
                            const std::string& accessor_prefix,
                            std::map<int32_t, std::string>* id_accessors, 
                            bool root) const {
        auto& accessors = *id_accessors;

        std::string code;
        if (const group* g = dynamic_cast<const group*>(n)) {
            if (root) accessors[g->get_id()] = "root";
            else accessors[g->get_id()] = accessor_prefix + n->get_name() + ".group";

            std::string new_prefix = root ? "" : accessor_prefix + n->get_name() + ".";

            std::string subcode = "";
            for (const node* c : *g) {
                if (c->get_name() == "group") {
                    throw generate_error("group cannot have a child of name group");
                }
                subcode += "\n";
                subcode += generate_node(c, new_prefix, id_accessors, false);
            }

            std::string children_array;
            for (const node* n : *g) {
                if (children_array.size() > 0) {
                    children_array += ", ";
                }
                std::string accessor = accessors[n->get_id()].substr(new_prefix.length(), 
                                                                     std::string::npos);
                children_array += "&" + accessor;
            }
            indent(children_array, 4);

            // add the group
            subcode += "\n\ntelegen::node* const children_[" + std::to_string(g->num_children()) + "] = {\n";
            subcode += children_array;
            subcode += "\n};";
            if (root) {
                subcode += "\ntelegen::group root = telegen::group(" +
                                std::to_string(g->get_id()) + ", \"" + g->get_name() + 
                                "\", \"" + g->get_pretty() +
                                "\", \"" + g->get_desc() + 
                                "\", \"" + g->get_schema() +
                                "\"," + std::to_string(g->get_version()) +
                                ", children_, " + std::to_string(g->num_children()) + ");";
                code += subcode;
                code += "\n";
            } else {
                subcode += "\ntelegen::group group = telegen::group(" +
                                std::to_string(g->get_id()) + ", \"" + g->get_name() + 
                                "\", \"" + g->get_pretty() +
                                "\", \"" + g->get_desc() + 
                                "\", \"" + g->get_schema() +
                                "\"," + std::to_string(g->get_version()) +
                                ", children_, " + std::to_string(g->num_children()) + ");";
                indent(subcode, 4);
                code += "struct {";
                code += subcode;
                code += "\n} " + g->get_name() + ";";
            }
        } else if (dynamic_cast<const variable*>(n) != nullptr) {
            const variable* v = dynamic_cast<const variable*>(n);
            std::string type = "telegen::variable<" + type_to_cpp_ident(v->get_type()) + ">";
            code += type + " " + v->get_name() + " = " + 
                           type + "(" + std::to_string(v->get_id()) + ", \"" + v->get_name() + 
                           "\", \"" + v->get_pretty() + "\", \"" + v->get_desc() + "\", &" + 
                           type_to_name(v->get_type()) + "_type);";

            accessors[v->get_id()] = accessor_prefix + v->get_name();
        } else if (dynamic_cast<const action*>(n) != nullptr) {
            const action* a = dynamic_cast<const action*>(n);
            std::string type = "telegen::action<" + type_to_cpp_ident(a->get_ret_type()) + 
                                    "," + type_to_cpp_ident(a->get_arg_type()) + ">";
            code += type + " " + a->get_name() +  " = " + type + 
                            "(" + std::to_string(a->get_id()) + ", \"" + 
                                a->get_name() + "\", \"" +
                                a->get_pretty() + "\", \"" + 
                                a->get_desc() + "\", &" + 
                                type_to_name(a->get_arg_type()) + "_type, &" +
                                type_to_name(a->get_ret_type()) + "_type);";

            accessors[a->get_id()] = accessor_prefix + a->get_name();
        }
        return code;
    }

    std::string
    generator::generate_tree(const node* root) const {
        std::map<int32_t, std::string> id_accessors;
        std::string subcode = generate_node(root, "", &id_accessors, true);

        std::string accessors;
        int32_t last_id = -1;
        for (auto it : id_accessors) {
            int32_t id = it.first;
            // put in blanks
            for (int32_t bid = last_id + 1; bid < id; bid++) {
                if (bid % 3 == 0) accessors += "\n";
                // put in a blank id
                accessors += " nullptr,";
            }
            // put in the non-blank
            if (id % 3 == 0) accessors += "\n";
            accessors += " &" + it.second + ",";
            last_id = id;
        }
        indent(accessors, 4);
        if (accessors.length() > 0) accessors += "\n";

        subcode += "\n";
        subcode += "size_t table_size = " + std::to_string(last_id + 1) + ";\n";
        subcode += "telegen::node* const node_table[" + std::to_string(last_id + 1) + "] = {";
        subcode += accessors; 
        subcode += "};\n";

        std::string code = "struct node_tree {\n";
        indent(subcode, 4);
        code += subcode;
        code += "\n};";
        return code;
    }

    std::string
    generator::generate_profile(const profile* p) const {
        std::string code = "struct " + p->get_name() + "_config {\n";
        for (const std::string& set_name : p->sets()) {
            const std::unordered_set<node*>& set = p->get(set_name);
            std::unordered_set<uint32_t> id_set;
            for (node* n : set) id_set.insert(n->get_id());

            std::string var = "const telegen::id_array<" + 
                std::to_string(id_set.size()) + "> " + set_name + " = {";
            bool first = true;
            for (int32_t id : id_set) {
                if (!first) var += ", ";
                if (first) first = false;

                var += std::to_string(id);
            }
            var += "};";

            indent(var, 4);
            code += var;
            code += "\n";
        }
        code += "};";
        return code;
    }

    generator::result
    generator::generate_target(const generator::target& t) const {
        std::string code =
            "#pragma once\n\n"
            "#include <telegen/types.hpp>\n"
            "#include <telegen/nodes.hpp>\n";

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
            std::string types_code = generate_types(t.tree_target);
            if (namespace_.length() > 0) indent(types_code, 4);
            code += types_code;

            std::string tree_code = generate_tree(t.tree_target);
            if (namespace_.length() > 0) indent(tree_code, 4);
            code += tree_code;
            code += "\n\n";
        }

        // try and figure out what tree this is being built against
        // either by looking at the tree include or at the tree_target
        const node* tr = nullptr;
        try {
            tr = t.tree_target ? t.tree_target :
                        targets_.at(t.tree_include).tree_target;
        } catch (std::out_of_range& e) {}

        if (!tr) throw missing_error("Could not find tree");

        for (const profile* p : t.profiles) {
            std::string subcode = generate_profile(p);
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
    generator::generate() const {
        std::vector<result> r;
        for (const auto& i : targets_) {
            r.push_back(generate_target(i.second));
        }
        return r;
    }
}
