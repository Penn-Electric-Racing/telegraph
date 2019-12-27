#ifndef __TELEGRAPH_GEN_TRAITS_HPP__
#define __TELEGRAPH_GEN_TRAITS_HPP__

#include "common.nanopb.h"

namespace telegen { 
    // this is overloaded to return 
    // the protobuffer type class in a constexpr manner
    // given the c++ type

    template<typename T>
        constexpr telegraph_proto_Type_Class get_type_class() {
            return telegraph_proto_Type_Class_ENUM;
        }

    template<>
        constexpr telegraph_proto_Type_Class get_type_class<void>() {
            return telegraph_proto_Type_Class_NONE;
        }

    template<>
        constexpr telegraph_proto_Type_Class get_type_class<bool>() {
            return telegraph_proto_Type_Class_BOOL;
        }


    // enum type

    template<typename T>
        struct type_info {
            using value_type = T;

            constexpr type_info(const char* nm, 
                    size_t nl, const char* const *l) : type_class(get_type_class<T>()),
                                                       name(nm), num_labels(nl), labels(l) {}

            telegraph_proto_Type_Class type_class;
            const char* name;
            uint8_t num_labels;
            const char* const *labels; // array of strings
        };

    struct empty {};

    template<>
        struct type_info<void> {
            using value_type = empty;

            constexpr type_info() {}

            telegraph_proto_Type_Class type_class = telegraph_proto_Type_Class_NONE;
            const char* name = "none";
            uint8_t num_labels = 0;
            const char** labels = {}; 
        };

}

// PRIMITIVE DATA TYPES (not in a namespace)

constexpr telegen::type_info<void> none_type = telegen::type_info<void>();
constexpr telegen::type_info<void> void_type = telegen::type_info<void>();

constexpr telegen::type_info<bool> bool_type = telegen::type_info<bool>("bool", 0, {});

constexpr telegen::type_info<int8_t> int8_type = telegen::type_info<int8_t>("int8", 0, {});

constexpr telegen::type_info<float> float_type = telegen::type_info<float>("float", 0, {});
constexpr telegen::type_info<double> double_type = telegen::type_info<double>("double", 0, {});
#endif
