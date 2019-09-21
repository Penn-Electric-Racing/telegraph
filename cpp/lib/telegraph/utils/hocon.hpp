#ifndef __TELEGRAPH_HOCON_PARSER_HPP__
#define __TELEGRAPH_HOCON_PARSER_HPP__

#include "json.hpp"

namespace telegraph {
    class node;

    class hocon_parser {
    public:
        json parse_file(const std::string& file);
    };
}

#endif
