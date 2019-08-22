#ifndef __PER_CONFIGURATION_HPP__
#define __PER_CONFIGURATION_HPP__

#include "utils/json_fwd.hpp"
#include "nodes/node.hpp"

namespace per {
    class config {
    public:
        // given a config json object, this will return a fully instantiated
        // configuration object including all codegen related information
        static std::unique_ptr<config> parse_config(const json& config);
    private:
    };
}
#endif
