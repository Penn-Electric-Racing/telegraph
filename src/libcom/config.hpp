#ifndef __PER_CONFIGURATION_HPP__
#define __PER_CONFIGURATION_HPP__

#include "json.hpp"
#include "nodes/node.hpp"

namespace per {
    class config {
    public:
        // configuration parsing code is here
        // given the root json object, this will 
        // return a parsed node tree
        static shared_group parse_root(const json& root);

        // given a config json object, this will return a fully instantiated
        // configuration object for a particular board
        static std::shared_ptr<config> parse_config(const json& config,
                                                    const shared_group& root);
    private:
        // specific to the configuration type
        json parameters_;
    };
}
#endif
