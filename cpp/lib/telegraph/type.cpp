#include "type.hpp"

namespace telegraph {
    std::string
    type::to_str() const {
        switch (class_) {
        case INVALID: return "invalid";
        case NONE: return "none";
        case BOOL: return "bool";
        case UINT8: return "uint8";
        case UINT16: return "uint16";
        case UINT32: return "uint32";
        case UINT64: return "uint64";
        case INT8: return "int8";
        case INT16: return "int16";
        case INT32: return "int32";
        case INT64: return "int64";
        case ENUM: {
            std::string s = "enum";
            if (name_.size() > 0) {
                s += "/";
                s += name_;
            }
            if (labels_.size() > 0) {
                s += " [";
                bool first = true;
                for (const std::string& l : labels_) {
                    if (!first) s += ", ";
                    s += l;
                    first = false;
                }
                s += "]";
            }
            return s;
        }
        default: return "unknown";
        }
    }
}
