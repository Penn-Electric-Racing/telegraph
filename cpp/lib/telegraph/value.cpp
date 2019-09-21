#include "value.hpp"

namespace telegraph {
    std::ostream& operator<<(std::ostream& o, const value& v) {
        switch (v.get_type_class()) {
        case type::INVALID: o << "invalid"; break;
        case type::NONE: o << "none"; break;
        case type::BOOL: o << (v.get<bool>() ? "true" : "false"); break;
        case type::ENUM: o << "enum(" << (int) v.get<uint8_t>() << ")"; break;
        case type::UINT8: o << (int) v.get<uint8_t>(); break;
        case type::UINT16: o << v.get<uint16_t>(); break;
        case type::UINT32: o << v.get<uint32_t>(); break;
        case type::UINT64: o << v.get<uint64_t>(); break;
        case type::INT8: o << (int) v.get<int8_t>(); break;
        case type::INT16: o << v.get<int16_t>(); break;
        case type::INT32: o << v.get<int32_t>(); break;
        case type::INT64: o << v.get<int64_t>(); break;
        case type::FLOAT: o << v.get<float>(); break;
        case type::DOUBLE: o << v.get<double>(); break;
        }
        return o;
    }
}
