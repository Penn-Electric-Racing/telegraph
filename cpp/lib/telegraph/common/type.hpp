#ifndef __TELEGRAPH_TYPE_HPP__
#define __TELEGRAPH_TYPE_HPP__

#include <vector>
#include <string>
#include <string_view>

#include "common.pb.h"

namespace telegraph {
    class value_type {
    public:
        enum type_class {
            Invalid, None, Enum,
            Bool,
            Uint8, Uint16, Uint32, Uint64,
            Int8, Int16, Int32, Int64,
            Float, Double
        };
        value_type() : class_(Invalid), name_(), labels_() {}
        value_type(type_class c) : class_(c), name_(), labels_() {}
        value_type(const std::string_view& name, const std::vector<std::string>& labels)
            : class_(Enum), name_(name), labels_(labels) {}
        value_type(const std::string_view& name, std::vector<std::string>&& labels)
            : class_(Enum), name_(name), labels_(std::move(labels)) {}

        constexpr type_class get_class() const { return class_; }
        constexpr const std::string& get_name() const { return name_; }
        constexpr const std::vector<std::string>& get_labels() const { return labels_; }

        void add_label(const std::string_view& label) { labels_.push_back(std::string{label}); }
        void add_label(const std::string& label) { labels_.push_back(label); }

        void set_class(type_class tc) { class_ = tc; }
        void set_labels(std::vector<std::string>&& labels) { labels_ = labels; }
        void set_name(const std::string& name) { name_ = name; }

        inline std::string to_str() const {
            switch (class_) {
                case Invalid: return "invalid";
                case None: return "none";
                case Bool: return "bool";
                case Uint8: return "uint8";
                case Uint16: return "uint16";
                case Uint32: return "uint32";
                case Uint64: return "uint64";
                case Int8: return "int8";
                case Int16: return "int16";
                case Int32: return "int32";
                case Int64: return "int64";
                case Enum: {
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
            case Invalid: return PClass::Type_Class_INVALID;
            case None: return PClass::Type_Class_NONE;
            case Enum: return PClass::Type_Class_ENUM;
            case Bool: return PClass::Type_Class_BOOL;
            case Uint8: return PClass::Type_Class_UINT8;
            case Uint16: return PClass::Type_Class_UINT16;
            case Uint32: return PClass::Type_Class_INT32;
            case Uint64: return PClass::Type_Class_UINT64;
            case Int8: return PClass::Type_Class_INT8;
            case Int16: return PClass::Type_Class_INT16;
            case Int32: return PClass::Type_Class_INT32;
            case Int64: return PClass::Type_Class_INT64;
            case Float: return PClass::Type_Class_FLOAT;
            case Double: return PClass::Type_Class_DOUBLE;
            default: return PClass::Type_Class_INVALID;
            }
        }
        static constexpr type_class unpack(Type::Class tc) {
            switch (tc) {
            case Type_Class_INVALID: return type_class::Invalid;
            case Type_Class_NONE: return type_class::None;
            case Type_Class_ENUM: return type_class::Enum;
            case Type_Class_BOOL: return type_class::Bool;
            case Type_Class_UINT8: return type_class::Uint8;
            case Type_Class_UINT16: return type_class::Uint16;
            case Type_Class_UINT32: return type_class::Uint32;
            case Type_Class_UINT64: return type_class::Uint64;
            case Type_Class_INT8: return type_class::Int8;
            case Type_Class_INT16: return type_class::Int16;
            case Type_Class_INT32: return type_class::Int32;
            case Type_Class_INT64: return type_class::Int64;
            default: return type_class::Invalid;
            }
        }

        void pack(Type* tc) const {
            auto l = tc->mutable_labels();
            for (auto& s : labels_) {
                *l->Add() = s;
            }
            tc->set_name(name_);
            tc->set_type(pack(class_));
        }
        inline static value_type unpack(const Type& tc) {
            value_type t;
            t.set_class(unpack(tc.type()));
            t.set_name(tc.name());
            std::vector<std::string> labels;
            for (int i = 0; i < tc.labels_size(); i++) {
                labels.push_back(tc.labels(i));
            }
            t.set_labels(std::move(labels));
            return t;
        }

        bool operator==(const value_type& other) const {{
            return class_ == other.class_ && name_ == other.name_
                && labels_ == other.labels_;
        }}
    private:
        type_class class_;
        std::string name_; // only set for enum types
        // contains the unit for this value_type
        // for for an enum the labels per value
        std::vector<std::string> labels_;
    };
}

#endif
