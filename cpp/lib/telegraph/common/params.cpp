#include "params.hpp"

#include "api.pb.h"
#include "namespace.hpp"
#include "nodes.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace telegraph {

    params
    params::unpack(const api::Params& i, namespace_* n) {
        switch (i.content_case()) {
        case api::Params::CONTENT_NOT_SET:
        case api::Params::kNone: return params();
        case api::Params::kNumber: return params(i.number());
        case api::Params::kB: return params(i.b());
        case api::Params::kStr: return params(i.str());
        case api::Params::kObject: {
            std::map<std::string, params, std::less<>> map;
            const auto& obj = i.object();
            for (int i = 0; i < obj.entries_size(); i++) {
                const api::ParamsEntry& entry = obj.entries(i);
                map.emplace(std::make_pair(entry.key(), params::unpack(entry.value(), n)));
            }
            return params(std::move(map));
        }
        case api::Params::kArray: {
            std::vector<params> vec;
            const auto& arr = i.array();
            for (int i = 0; i < arr.elements_size(); i++) {
                vec.push_back(params::unpack(arr.elements(i), n));
            }
            return params(std::move(vec));
        }
        case api::Params::kCtxUuid: {
            if (!n) return params();
            uuid u = boost::lexical_cast<uuid>(i.ctx_uuid());
            auto ctx = n->contexts->get(u);
            if (!ctx) return params();
            return params(ctx);
        }
        case api::Params::kCompUuid: {
            if (!n) return params();
            uuid u = boost::lexical_cast<uuid>(i.ctx_uuid());
            auto comp = n->components->get(u);
            if (!comp) return params();
            return params(comp);
        }
        case api::Params::kTree: {
            return params(std::shared_ptr<node>{node::unpack(i.tree())});
        }
        }
        return params();
    }

    void
    params::pack(api::Params* i) const {
        std::size_t index = value_.index();
        switch (index) {
        default: 
        case 0: i->mutable_none(); break;
        case 1: i->set_number(std::get<float>(value_)); break;
        case 2: i->set_number(std::get<bool>(value_)); break;
        case 3: i->set_str(std::get<std::string>(value_)); break;
        case 4: {
            const std::map<std::string, params, std::less<>>& o = std::get<std::map<std::string, params, std::less<>>>(value_);
            api::ParamsMap* m = i->mutable_object();
            for (const auto& p : o) {
                api::ParamsEntry* e = m->add_entries();
                e->set_key(p.first);
                api::Params* in = e->mutable_value();
                p.second.pack(in);
            }
        } break;
        case 5: {
            const std::vector<params>& v = std::get<std::vector<params>>(value_);
            api::ParamsList* l = i->mutable_array();
            for (const params& in : v) {
                api::Params* e = l->add_elements();
                in.pack(e);
            }
        } break;
        case 6: {
            const context_ptr& c = std::get<context_ptr>(value_);
            if (!c) i->mutable_none();
            else i->set_ctx_uuid(boost::lexical_cast<std::string>(c->get_uuid()));
        } break;
        case 7: {
            const component_ptr& c = std::get<component_ptr>(value_);
            if (!c) i->mutable_none();
            else i->set_comp_uuid(boost::lexical_cast<std::string>(c->get_uuid()));
        } break;
        case 8: {
            const std::shared_ptr<node>& n = std::get<std::shared_ptr<node>>(value_);
            if (!n) {
                i->mutable_none();
            } else {
                n->pack(i->mutable_tree());
            }
        } break;
        }
    }

    void
    params::move(api::Params* i) {
        std::size_t index = value_.index();
        switch (index) {
        default: 
        case 0: i->mutable_none(); break;
        case 1: i->set_number(std::get<float>(value_)); break;
        case 2: i->set_b(std::get<bool>(value_)); break;
        case 3: i->set_str(std::move(std::get<std::string>(value_))); break;
        case 4: {
            std::map<std::string, params, std::less<>>& o = std::get<std::map<std::string, params, std::less<>>>(value_);
            api::ParamsMap* m = i->mutable_object();
            for (auto& p : o) {
                api::ParamsEntry* e = m->add_entries();
                e->set_key(p.first);
                api::Params* in = e->mutable_value();
                p.second.move(in);
            }
        } break;
        case 5: {
            std::vector<params>& v = std::get<std::vector<params>>(value_);
            api::ParamsList* l = i->mutable_array();
            for (params& in : v) {
                api::Params* e = l->add_elements();
                in.move(e);
            }
        } break;
        }
    }
}
