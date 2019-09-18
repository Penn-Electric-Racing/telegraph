#include "tree.hpp"

#include "group.hpp"
#include "variable.hpp"
#include "action.hpp"
#include "node.hpp"

#include "../type.hpp"
#include "../errors.hpp"

#include "../utils/json.hpp"

#include <sstream>
#include <string>
#include <iostream>
#include <cassert>

namespace per {
    tree::tree() : root_() {}
    tree::tree(group* root) : root_(root) {
        assert(root != nullptr);
        root_->on_descendant_added.add(this, [this] (node* n) { on_descendant_added(n); });
        root_->on_descendant_removed.add(this, [this] (node* n) { on_descendant_removed(n); });
    }

    tree::~tree() {
        on_dispose();
        root_->on_descendant_added.remove(this);
        root_->on_descendant_removed.remove(this);
        delete root_;
    }

    std::ostream& operator<<(std::ostream& o, const tree& t) {
        o << *t.get_root();
        return o;
    }

    std::vector<node*>
    tree::nodes(bool postorder) {
        if (!root_) return std::vector<node*>();
        else return root_->descendants(true, postorder);
    }

    std::vector<const node*>
    tree::nodes(bool postorder) const {
        if (!root_) return std::vector<const node*>();
        else return ((const node*) root_)->descendants(true, postorder);
    }

    // UNPACKING CODE

    static std::unordered_map<std::string, type::type_class> s_type_map = {
        {"invalid", type::INVALID},
        {"none", type::NONE},
        {"enum", type::ENUM},
        {"bool", type::BOOL},
        {"uint8", type::UINT8},
        {"uint16", type::UINT16},
        {"uint32", type::UINT32},
        {"uint64", type::UINT64},
        {"int8", type::INT8},
        {"int16", type::INT16},
        {"int32", type::INT32},
        {"int64", type::INT64},
        {"float", type::FLOAT},
        {"double", type::DOUBLE}
    };

    static node* unpack_node(const std::string& name, const json& json);

    static
    type::type_class unpack_type_class(const std::string& tc) {
        try { 
            return s_type_map.at(tc);
        } catch (const std::out_of_range& r) {
            return type::INVALID;
        }
    }

    static
    type unpack_type(const json& json) {
        type t(type::INVALID);
        if (json.is_object()) {
            t = type(unpack_type_class(json.value("type", "invalid")));
            if (t.get_class() == type::ENUM) {
                std::vector<std::string> strs = json.value<std::vector<std::string>>("labels", {});
                t.set_labels(strs);
            }
        } else if (json.is_string()) {
            t = type(unpack_type_class(json.get<std::string>()));
        } else throw parse_error("Unable to parse type: " + json.dump());
        return t;
    }

    static
    variable* unpack_variable(const std::string& name, const json& json) {
        type t = unpack_type(json);
        std::string pretty = json.is_object() ? json.value("pretty", "") : "";
        std::string desc = json.is_object() ? json.value("desc", "") : "";
        return new variable(name, pretty, desc, unpack_type(json));
    }

    static
    action* unpack_action(const std::string& name, const json& json) {
        type arg(type::NONE);
        type ret(type::NONE);
        if (json.is_object()) {
            if (json.find("arg") != json.end()) arg = unpack_type(json["arg"]);
            if (json.find("ret") != json.end()) ret = unpack_type(json["ret"]);
        } 
        std::string pretty = json.is_object() ? json.value("pretty", "") : "";
        std::string desc = json.is_object() ? json.value("desc", "") : "";

        return new action(name, pretty, desc, arg, ret);
    }

    static
    action* unpack_stream(const std::string& name, const json& json) {
        throw parse_error("Cannot parse stream");
    }


    static
    group* unpack_group(const std::string& name, const json& json) {
        std::string schema = json.value("schema", "none");
        int version = json.value("version", 0);
        std::string pretty = json.value("pretty", "");
        std::string desc = json.value("desc", "");

        group* g = new group(name, pretty, desc, schema, version);

        for (const auto& item : json.items()) {
            const auto& key = item.key();
            if (key != "schema" && key != "version" &&
                key != "pretty" && key != "desc" && key != "type") {

                g->add_child(unpack_node(key, item.value()));
            }
                
        }
        return g;
    }

    static
    node* unpack_node(const std::string& name, const json& json) {
        std::string type;
        if (json.is_string()) {
            type = json.get<std::string>();
        } else if (json.is_object()) {
            type = json.value("type", "group");
        } else throw parse_error("Unable to parse: " + json.dump());

        if (type == "group") {
            return unpack_group(name, json);
        } else if (type == "action") {
            return unpack_action(name, json);
        } else if (type == "stream") {
            return unpack_stream(name, json);
        } else {
            return unpack_variable(name, json);
        }
    }

    tree*
    tree::unpack(const json& json_root) {
        group* root = new group("root", "", "", "root", json_root.value("version", 1));
        for (const auto& item : json_root.items()) {
            if (item.key() == "version") continue;
            root->add_child(unpack_node(item.key(), item.value()));
        }
        return new tree(root);
    }
}
