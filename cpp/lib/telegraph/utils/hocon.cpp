#include "hocon.hpp"

#include "../errors.hpp"

#include <hocon/config_parse_options.hpp>
#include <hocon/config.hpp>

namespace telegraph {

    /*
    static json parse_value(const hocon::shared_value& val);

    static json parse_object(const hocon::shared_object& obj) {
        std::unordered_map<std::string, json> map;
        for (auto entry : *obj) {
            map[entry.first] = parse_value(entry.second);
        }
        return json(map);
    }

    static json parse_list(const hocon::shared_list& list) {
        std::vector<json> objects;
        for (hocon::shared_value val : *list) {
            objects.push_back(parse_value(val));
        }
        return json(objects);
    }

    static json parse_number(hocon::unwrapped_value unwrapped) {
        if (unwrapped.type() == typeid(int)) {
            return json(boost::get<int>(unwrapped));
        } else if (unwrapped.type() == typeid(uint64_t)) {
            return json(boost::get<int64_t>(unwrapped));
        } else if (unwrapped.type() == typeid(double)) {
            return json(boost::get<double>(unwrapped));
        } else {
            return json(boost::get<double>(unwrapped));
        }
    }

    static json parse_boolean(hocon::unwrapped_value unwrapped) {
        return json(boost::get<bool>(unwrapped));
    }

    static json parse_string(hocon::unwrapped_value unwrapped) {
        return json(boost::get<std::string>(unwrapped));
    }

    static json parse_null() { return json(); }

    static json parse_value(const hocon::shared_value& val) {
        switch(val->value_type()) {
            case hocon::config_value::type::OBJECT: return parse_object((const hocon::shared_object&) val);
            case hocon::config_value::type::LIST: return parse_list((const hocon::shared_list&) val);
            case hocon::config_value::type::NUMBER: return parse_number(val->unwrapped());
            case hocon::config_value::type::BOOLEAN: return parse_boolean(val->unwrapped());
            case hocon::config_value::type::CONFIG_NULL: return parse_null();
            case hocon::config_value::type::STRING: return parse_string(val->unwrapped());
            case hocon::config_value::type::UNSPECIFIED: throw std::logic_error("Got unspecified hocon object");
        }
        return json();
    }*/

    json
    hocon_parser::parse_file(const std::string& file) {
        try {
            hocon::config_parse_options opts = 
                hocon::config_parse_options::defaults().set_allow_missing(false);
            hocon::shared_config doc = hocon::config::parse_file_any_syntax(file, opts);
            doc = doc->resolve();
            json val = doc->root()->unwrapped(); 
            return val;
        } catch (const hocon::config_exception& e) {
            throw parse_error(e.what());
        }
        return json();
    }
}
