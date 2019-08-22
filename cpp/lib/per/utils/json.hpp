#ifndef __PER_JSON_HPP__
#define __PER_JSON_HPP__

#include <nlohmann/json.hpp>

namespace per {
    using json = nlohmann::json;

    class json_parser {
    public:
        json parse_file(const std::string& file);
    };
}

#endif

