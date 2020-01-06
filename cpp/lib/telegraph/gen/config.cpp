#include "config.hpp"

#include "../utils/errors.hpp"
#include "../utils/json.hpp"

#include <sstream>
#include <iostream>

namespace telegraph {
    static void unpack_profile(profile* p, 
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
                std::string path = n->topic();
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
            p->add(name, std::move(set));
        }
    }

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
    static
    type::type_class unpack_type_class(const std::string& tc) {
        try { 
            return s_type_map.at(tc);
        } catch (const std::out_of_range& r) {
            return type::INVALID;
        }
    }
    static node* unpack_node(int32_t* id_counter, const std::string& name, const json& json);

    static type unpack_type(const json& json) {
        type t(type::INVALID);
        if (json.is_object()) {
            t = type(unpack_type_class(json.value("type", "invalid")));
            if (t.get_class() == type::ENUM) {
                std::vector<std::string> strs = json.value<std::vector<std::string>>("labels", {});
                t.set_labels(std::move(strs));
                if (json.find("type_name") == json.end())
                    throw parse_error("enum type expects type_name: " + json.dump());
                t.set_name(json["type_name"]);
            }
        } else if (json.is_string()) {
            t = type(unpack_type_class(json.get<std::string>()));
        } else throw parse_error("unable to parse type: " + json.dump());
        return t;
    }

    static variable* unpack_variable(int32_t* id_counter, const std::string& name, 
                                     const json& json) {
        type t = unpack_type(json);
        std::string pretty = json.is_object() ? json.value("pretty", "") : "";
        std::string desc = json.is_object() ? json.value("desc", "") : "";
        return new variable((*id_counter)++, name, pretty, desc, t);
    }

    static action* unpack_action(int32_t* id_counter, const std::string& name, 
                                 const json& json) {
        type arg(type::NONE);
        type ret(type::NONE);
        if (json.find("arg") != json.end()) arg = unpack_type(json["arg"]);
        if (json.find("ret") != json.end()) ret = unpack_type(json["ret"]);
        std::string pretty = json.value("pretty", "");
        std::string desc = json.value("desc", "");
        return new action((*id_counter)++, name, pretty, desc, arg, ret);
    }

    static group* unpack_group(int32_t* id_counter, const std::string& name,
                                const json& json) {
        int32_t id = (*id_counter)++;
        std::string schema = json.value("schema", "none");
        int version = json.value("version", 0);
        std::string pretty = json.value("pretty", "");
        std::string desc = json.value("desc", "");
        std::vector<node*> children;
        for (const auto& item : json.items()) {
            const auto& key = item.key();
            if (key != "schema" && key != "version" &&
                 key != "pretty" && key != "desc" && key != "type") {
                children.push_back(unpack_node(id_counter, key, item.value()));
            }
        }
        return new group(id, name, pretty, desc, schema, version, std::move(children));
    }

    static node* unpack_node(int32_t* id_counter, const std::string& name, 
                             const json& json) {
        if (json.is_string()) return unpack_variable(id_counter, name, json);
        if (!json.is_object()) throw parse_error("unable to parse node: " + json.dump());

        std::string type = json.value("type", "group");
        if (type == "group") {
            return unpack_group(id_counter, name, json);
        } else if (type == "action") {
            return unpack_action(id_counter, name, json);
        } else if (type == "stream") {
            throw parse_error("cannot unpack stream yet!");
        } else {
            return unpack_variable(id_counter, name, json);
        }
    }

    config::config(const json& j) : tree_(nullptr), profiles_() {
        int32_t id_counter = 0;
        tree_ = unpack_node(&id_counter, "root", j["root"]);

        std::vector<node*> nodes = tree_->nodes();
        if (j.find("configs") != j.end()) {
            auto& c = j["configs"];
            for (const auto& item : c.items()) {
                std::string name = item.key();
                profile p = profile(name);
                unpack_profile(&p, nodes, item.value());
                profiles_.emplace(std::make_pair(name, std::move(p)));
            }
        }
    }

    config::~config() {
        delete tree_;
    }
}
