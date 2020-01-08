#ifndef __TELEGRAPH_TYPE_HPP__
#define __TELEGRAPH_TYPE_HPP__

#include <vector>
#include <string>

#include "common.pb.h"

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

        inline void set_class(type_class tc) { class_ = tc; }
        inline void set_labels(std::vector<std::string>&& labels) { labels_ = labels; }
        inline void set_name(const std::string& name) { name_ = name; }

        inline std::string to_str() const {
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

        static constexpr Type::Class pack(type_class tc) {
            using PClass = Type::Class;
            switch (tc) {
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
        static constexpr type_class unpack(Type::Class tc) {
            switch (tc) {
            case Type_Class_INVALID: return type_class::INVALID;
            default: return type_class::INVALID;
            }
        }

        inline void pack(Type* tc) const {
            auto l = tc->mutable_labels();
            for (auto& s : labels_) {
                *l->Add() = s;
            }
            tc->set_name(name_);
            tc->set_type(pack(class_));
        }
        inline static type unpack(const Type& tc) {
            type t;
            t.set_class(unpack(tc.type()));
            t.set_name(tc.name());
            std::vector<std::string> labels;
            for (int i = 0; i < tc.labels_size(); i++) {
                labels.push_back(tc.labels(i));
            }
            t.set_labels(std::move(labels));
            return t;
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
