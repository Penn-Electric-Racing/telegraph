#include "json.hpp"

#include <fstream>

namespace per {
    json
    json_parser::parse_file(const std::string& file) {
        std::ifstream s(file);
        json j;
        s >> j;
        return j;
    }
}
