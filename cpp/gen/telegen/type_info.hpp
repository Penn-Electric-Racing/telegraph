#ifndef __TELEGRAPH_GEN_TRAITS_HPP__
#define __TELEGRAPH_GEN_TRAITS_HPP__

#include "util.hpp"

#include "common.nanopb.h"

// declared outside namespace
struct none {};

namespace telegen { 
    // this is overloaded to return 
    // the protobuffer type class in a constexpr manner
    // given the c++ type

    template<typename T>
        constexpr telegraph_Type_Class get_type_class() {
            return telegraph_Type_Class_ENUM;
        }

    template<>
        constexpr telegraph_Type_Class get_type_class<none>() {
            return telegraph_Type_Class_NONE;
        }

    template<>
        constexpr telegraph_Type_Class get_type_class<bool>() {
            return telegraph_Type_Class_BOOL;
        }


    // enum type

    template<typename T>
        struct type_info {
            using value_type = T;

            constexpr type_info(const char* nm, 
                    size_t nl, const char* const *l) : 
                type_class(get_type_class<T>()),
                name(nm), num_labels(nl), labels(l) {}

            telegraph_Type_Class type_class;
            const char* name;
            uint8_t num_labels;
            const char* const *labels; // array of strings

            void pack(telegraph_Type* type) const {
                type->type = type_class;
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

constexpr telegen::type_info<int8_t> int8_type = telegen::type_info<int8_t>("int8", 0, {});

constexpr telegen::type_info<float> float_type = telegen::type_info<float>("float", 0, {});
constexpr telegen::type_info<double> double_type = telegen::type_info<double>("double", 0, {});
#endif
