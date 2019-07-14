#include "config.hpp"

#include "nodes/node.hpp"
#include "nodes/group.hpp"

namespace per {
    std::shared_ptr<group>
    config::parse_root(const json& json_root) {
        shared_group root = std::make_shared<group>();
        root->set_name("root");
        root->set_schema("root");
        root->set_version(1);

        return root;
    }

    std::shared_ptr<config>
    config::parse_config(const json& conf, const shared_group& root) {
        return std::shared_ptr<config>();
    }
}
