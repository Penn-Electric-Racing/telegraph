#ifndef __PER_HOCON_PARSER_HPP__
#define __PER_HOCON_PARSER_HPP__

#include "json.hpp"

namespace per {
    class node;

    class hocon_parser {
    public:
        json parse_file(const std::string& file);
    };
}

#endif
