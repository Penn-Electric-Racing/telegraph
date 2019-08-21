#ifndef __PER_CONFIGURATION_HPP__
#define __PER_CONFIGURATION_HPP__

#include "utils/json_fwd.hpp"
#include "nodes/node.hpp"

namespace per {
    class config {
    public:
        // given a config json object, this will return a fully instantiated
        // configuration object for a particular board
        static std::shared_ptr<config> parse_config(const json& config,
                                                    const shared_group& root);
    private:
    };
}
#endif
