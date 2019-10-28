#ifndef __TELEGRAPH_TYPE_HPP__
#define __TELEGRAPH_TYPE_HPP__

#include <vector>
#include <string>

#include "api.pb.h"

namespace telegraph {
    class type {
    public:
        enum type_class {
            INVALID, NONE, ENUM,
            BOOL,
            UINT8, UINT16, UINT32, UINT64,
            INT8, INT16, INT32, INT64,
            FLOAT, DOUBLE
        };
        inline type() : class_(INVALID), name_(), labels_() {}
        inline type(type_class c) : class_(c), name_(), labels_() {}

        constexpr type_class get_class() const { return class_; }
        constexpr const std::string& get_name() const { return name_; }
        constexpr const std::vector<std::string>& get_labels() const { return labels_; }

        inline void add_label(const std::string& label) { labels_.push_back(label); }
        inline void set_labels(const std::vector<std::string> labels) { labels_ = labels; }

        inline void set_name(const std::string& name) { name_ = name; }

        std::string to_str() const;

        constexpr const proto::Type::Class to_proto_buffer_class() const {
            using PClass = proto::Type::Class;
            switch (class_) {
            case INVALID: return PClass::Type_Class_INVALID;
            case NONE: return PClass::Type_Class_NONE;
            case ENUM: return PClass::Type_Class_ENUM;
            case BOOL: return PClass::Type_Class_BOOL;
            case UINT8: return PClass::Type_Class_UINT8;
            case UINT16: return PClass::Type_Class_UINT16;
            case UINT32: return PClass::Type_Class_INT32;
            case UINT64: return PClass::Type_Class_UINT64;
            case INT8: return PClass::Type_Class_INT8;
            case INT16: return PClass::Type_Class_INT16;
            case INT32: return PClass::Type_Class_INT32;
            case INT64: return PClass::Type_Class_INT64;
            case FLOAT: return PClass::Type_Class_FLOAT;
            case DOUBLE: return PClass::Type_Class_DOUBLE;
            default: return PClass::Type_Class_INVALID;
            }
        }
    private:
        type_class class_;
        std::string name_; // only set for enum types
        // contains the unit for this type
        // for for an enum the labels per value
        std::vector<std::string> labels_;
    };
}

#endif
