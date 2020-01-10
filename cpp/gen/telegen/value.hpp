#ifndef __TELEGEN_VALUE_HPP__
#define __TELEGEN_VALUE_HPP__

#include "types.hpp"
#include "common.nanopb.h"

namespace telegen {
    class value;

    class value {
    public:
        union box {
            bool b;
            uint8_t uint8;
            uint16_t uint16;
            uint32_t uint32;
            uint64_t uint64;
            int8_t int8; // also used for the enum type
            int16_t int16;
            int32_t int32;
            int64_t int64;
            float f;
            double d;
        }; 

        constexpr value() : type_(type_class::Invalid), value_() {}
        constexpr value(type_class t) : type_(t), value_() {}

        constexpr type_class get_type_class() const { return type_; }
        constexpr const box& get_box() const { return value_; }
        constexpr box& get_box() { return value_; }

        template<typename T>
            constexpr const T &get() const {
                return (const T&) value_;
            }
        template<typename T>
            constexpr T &get() {
                return (T&) value_;
            }
        template<typename T>
            constexpr void set(const T& v) {
                return get<T>() = v;
            }

        void pack(telegraph_Value* val) const {
            switch (type_) {
            case type_class::None: {
                val->which_type = telegraph_Value_none_tag; 
                val->type.none = telegraph_Empty_init_default;
            } break;
            case type_class::Enum: {
                val->which_type = telegraph_Value_en_tag;
                val->type.en = value_.uint8;
            } break;
            case type_class::Bool: {
                val->which_type = telegraph_Value_b_tag;
                val->type.b = value_.b;
            } break;
            case type_class::Uint8: {
                val->which_type = telegraph_Value_u8_tag;
                val->type.u8 = value_.uint8;
            } break;
            case type_class::Uint16: {
                val->which_type = telegraph_Value_u16_tag;
                val->type.u16 = value_.uint16;
            } break;
            case type_class::Uint32: {
                val->which_type = telegraph_Value_u32_tag;
                val->type.u32 = value_.uint32;
            } break;
            case type_class::Uint64: {
                val->which_type = telegraph_Value_u64_tag;
                val->type.u64 = value_.uint64;
            } break;
            case type_class::Int8: {
                val->which_type = telegraph_Value_i8_tag;
                val->type.i8 = value_.int8;
            } break;
            case type_class::Int16: {
                val->which_type = telegraph_Value_i16_tag;
                val->type.i16 = value_.int16;
            } break;
            case type_class::Int32: {
                val->which_type = telegraph_Value_i32_tag;
                val->type.i32 = value_.int32;
            } break;
            case type_class::Int64: {
                val->which_type = telegraph_Value_i64_tag;
                val->type.i64 = value_.int64;
            } break;
            case type_class::Float: {
                val->which_type = telegraph_Value_f_tag;
                val->type.f = value_.f;
            } break;
            case type_class::Double: {
                val->which_type = telegraph_Value_d_tag;
                val->type.d = value_.d;
            } break;
            }
        }
    private:
        type_class type_;
        box value_;
    };

}

#endif
