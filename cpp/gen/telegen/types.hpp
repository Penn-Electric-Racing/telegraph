#ifndef __TELEGEN_TYPES_HPP__
#define __TELEGEN_TYPES_HPP__

#include "util.hpp"

#include "common.nanopb.h"

#include <type_traits>

// declared outside namespace
struct none {};

namespace telegen { 
    // this is overloaded to return 
    // the protobuffer type class in a constexpr manner
    // given the c++ type
    enum class type_class {
        Invalid, None, Enum, Bool, 
        Uint8, Uint16, Uint32, Uint64,
        Int8,   Int16,  Int32,  Int64,
        Float, Double 
    };

    template<typename T>
    constexpr type_class get_type_class() { return type_class::Enum; }

    template<>
    constexpr type_class get_type_class<none>() { return type_class::None; }
    template<>
    constexpr type_class get_type_class<bool>() { return type_class::Bool; }
    template<>
    constexpr type_class get_type_class<uint8_t>() { return type_class::Uint8; }
    template<>
    constexpr type_class get_type_class<uint16_t>() { return type_class::Uint16; }
    template<>
    constexpr type_class get_type_class<uint32_t>() { return type_class::Uint32; }
    template<>
    constexpr type_class get_type_class<int8_t>() { return type_class::Int8; }
    template<>
    constexpr type_class get_type_class<int16_t>() { return type_class::Int16; }
    template<>
    constexpr type_class get_type_class<int32_t>() { return type_class::Int32; }
    template<>
    constexpr type_class get_type_class<int64_t>() { return type_class::Int64; }
    template<>
    constexpr type_class get_type_class<float>() { return type_class::Float; }
    template<>
    constexpr type_class get_type_class<double>() { return type_class::Double; }

    constexpr telegraph_Type_Class to_proto_type_class(type_class c) {
        switch (c) {
        case type_class::Invalid: return telegraph_Type_Class_INVALID;
        case type_class::Bool: return telegraph_Type_Class_BOOL;
        case type_class::Uint8: return telegraph_Type_Class_UINT8;
        case type_class::Uint16: return telegraph_Type_Class_UINT16;
        case type_class::Uint32: return telegraph_Type_Class_UINT32;
        case type_class::Uint64: return telegraph_Type_Class_UINT64;
        case type_class::Int8: return telegraph_Type_Class_INT8;
        case type_class::Int16: return telegraph_Type_Class_INT16;
        case type_class::Int32: return telegraph_Type_Class_INT32;
        case type_class::Int64: return telegraph_Type_Class_INT64;
        case type_class::Float: return telegraph_Type_Class_FLOAT;
        case type_class::Double: return telegraph_Type_Class_DOUBLE;
        default: return telegraph_Type_Class_INVALID;
        }
    }

    // enum type

    template<typename T>
        struct type_info {
            using value_type = T;

            constexpr type_info(const char* nm, 
                    size_t nl, const char* const *l) : 
                class_(get_type_class<T>()),
                name(nm), num_labels(nl), labels(l) {}

            type_class class_;
            const char* name;
            uint8_t num_labels;
            const char* const *labels; // array of strings

            void pack(telegraph_Type* type) const {
                type->type = to_proto_type_class(class_);
                type->name.arg = (void*) name;
                type->name.funcs.encode = util::proto_string_encoder;
                type->labels.arg = (void*) this;
                type->labels.funcs.encode =
                    [](pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg) {
                        const type_info<T>* t = (const type_info<T>*) *arg;
                        for (uint8_t i = 0; i < t->num_labels; i++) {
                            if (!pb_encode_tag_for_field(stream, field))
                                    return false;
                            const char* str = t->labels[i];
                            if (!pb_encode_string(stream, (uint8_t*) str, 
                                    strlen(str)))
                                return false;
                        }
                        return true;
                    };
            }
        };
}

// PRIMITIVE DATA TYPES (note: not in a namespace)
constexpr telegen::type_info<none> none_type = telegen::type_info<none>("none", 0, {});
constexpr telegen::type_info<bool> bool_type = telegen::type_info<bool>("bool", 0, {});

constexpr telegen::type_info<uint8_t> uint8_type =
            telegen::type_info<uint8_t>("uint8", 0, {});
constexpr telegen::type_info<uint16_t> uint16_type =
            telegen::type_info<uint16_t>("uint16", 0, {});
constexpr telegen::type_info<uint32_t> uint32_type =
            telegen::type_info<uint32_t>("uint32", 0, {});
constexpr telegen::type_info<uint64_t> uint64_type =
            telegen::type_info<uint64_t>("uint64", 0, {});

constexpr telegen::type_info<int8_t> int8_type =
            telegen::type_info<int8_t>("int8", 0, {});
constexpr telegen::type_info<int16_t> int16_type =
            telegen::type_info<int16_t>("int16", 0, {});
constexpr telegen::type_info<int32_t> int32_type =
            telegen::type_info<int32_t>("int32", 0, {});
constexpr telegen::type_info<int64_t> int64_type =
            telegen::type_info<int64_t>("int64", 0, {});

constexpr telegen::type_info<float> float_type = telegen::type_info<float>("float", 0, {});
constexpr telegen::type_info<double> double_type = telegen::type_info<double>("double", 0, {});
#endif
