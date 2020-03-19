#include "params.hpp"

#include "api.pb.h"

namespace telegraph {

    params::params(const api::Params& i) : value_() {
        switch (i.content_case()) {
        case api::Params::CONTENT_NOT_SET:
        case api::Params::kNone: break; // nothing to do, already null!
        case api::Params::kNumber: value_ = i.number(); break;
        case api::Params::kStr: value_ = i.str(); break;
        case api::Params::kObject: {
            std::map<std::string, params> map;
            const auto& obj = i.object();
            for (int i = 0; i < obj.entries_size(); i++) {
                const api::ParamsEntry& entry = obj.entries(i);
                map.emplace(std::make_pair(entry.key(), params{entry.value()}));
            }
            value_ = std::move(map);
        } break;
        case api::Params::kArray: {
            std::vector<params> vec;
            const auto& arr = i.array();
            for (int i = 0; i < arr.elements_size(); i++) {
                vec.push_back(arr.elements(i));
            }
            value_ = std::move(vec);
        } break;
        }
    }

    void
    params::pack(api::Params* i) const {
        std::size_t index = value_.index();
        switch (index) {
        default: 
        case 0: i->mutable_none(); break;
        case 1: i->set_number(std::get<float>(value_)); break;
        case 2: i->set_str(std::get<std::string>(value_)); break;
        case 3: {
            const std::map<std::string, params>& o = std::get<std::map<std::string, params>>(value_);
            api::ParamsMap* m = i->mutable_object();
            for (const auto& p : o) {
                api::ParamsEntry* e = m->add_entries();
                e->set_key(p.first);
                api::Params* in = e->mutable_value();
                p.second.pack(in);
            }
        } break;
        case 4: {
            const std::vector<params>& v = std::get<std::vector<params>>(value_);
            api::ParamsList* l = i->mutable_array();
            for (const params& in : v) {
                api::Params* e = l->add_elements();
                in.pack(e);
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
        case 2: i->set_str(std::move(std::get<std::string>(value_))); break;
        case 3: {
            std::map<std::string, params>& o = std::get<std::map<std::string, params>>(value_);
            api::ParamsMap* m = i->mutable_object();
            for (auto& p : o) {
                api::ParamsEntry* e = m->add_entries();
                e->set_key(p.first);
                api::Params* in = e->mutable_value();
                p.second.move(in);
            }
        } break;
        case 4: {
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
