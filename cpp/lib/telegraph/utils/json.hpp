#ifndef __TELEGRAPH_JSON_HPP__
#define __TELEGRAPH_JSON_HPP__

#include <nlohmann/json.hpp>

namespace telegraph {
    using json = nlohmann::json;

    class json_parser {
    public:
        json parse_file(const std::string& file);
    };
}

#endif

