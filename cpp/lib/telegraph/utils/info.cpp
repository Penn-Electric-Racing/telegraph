#include "info.hpp"

#include "api.pb.h"

namespace telegraph {

    info::info(const api::Info& i) : value_() {
        switch (i.content_case()) {
        case api::Info::CONTENT_NOT_SET:
        case api::Info::kNone: break; // nothing to do, already null!
        case api::Info::kNumber: value_ = i.number(); break;
        case api::Info::kStr: value_ = i.str(); break;
        case api::Info::kObject: {
            std::map<std::string, info> map;
            const auto& obj = i.object();
            for (int i = 0; i < obj.entries_size(); i++) {
                const api::InfoEntry& entry = obj.entries(i);
                map.emplace(std::make_pair(entry.key(), info{entry.value()}));
            }
            value_ = std::move(map);
        } break;
        case api::Info::kArray: {
            std::vector<info> vec;
            const auto& arr = i.array();
            for (int i = 0; i < arr.elements_size(); i++) {
                vec.push_back(arr.elements(i));
            }
            value_ = std::move(vec);
        } break;
        }
    }

    void
    info::pack(api::Info* i) const {
        std::size_t index = value_.index();
        switch (index) {
        default: 
        case 0: i->mutable_none(); break;
        case 1: i->set_number(std::get<float>(value_)); break;
        case 2: i->set_str(std::get<std::string>(value_)); break;
        case 3: {
            const std::map<std::string, info>& o = std::get<std::map<std::string, info>>(value_);
            api::InfoMap* m = i->mutable_object();
            for (const auto& p : o) {
                api::InfoEntry* e = m->add_entries();
                e->set_key(p.first);
                api::Info* in = e->mutable_value();
                p.second.pack(in);
            }
        } break;
        case 4: {
            const std::vector<info>& v = std::get<std::vector<info>>(value_);
            api::InfoList* l = i->mutable_array();
            for (const info& in : v) {
                api::Info* e = l->add_elements();
                in.pack(e);
            }
        } break;
        }
    }
}
