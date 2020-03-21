#ifndef __TELEGRAPH_VALUE_HPP__
#define __TELEGRAPH_VALUE_HPP__

#include <string>
#include <cinttypes>
#include <ostream>

#include "type.hpp"

#include "common.pb.h"

namespace telegraph {
    class value;

    template<typename T>
        T unwrap(const value& v) {}

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

        constexpr value() : type_(type::None), value_() {}
        constexpr value(bool b) : type_(type::Bool), value_() { value_.b = b; }
        constexpr value(uint8_t v) : type_(type::Uint8), value_() { value_.uint8 = v; }
        constexpr value(uint16_t v) : type_(type::Uint16), value_() { value_.uint16 = v; }
        constexpr value(uint32_t v) : type_(type::Uint32), value_() { value_.uint32 = v; }
        constexpr value(uint64_t v) : type_(type::Uint64), value_() { value_.uint64 = v; }

        constexpr value(int8_t v) : type_(type::Int8), value_() { value_.int8 = v; }
        constexpr value(int16_t v) : type_(type::Int16), value_() { value_.int16 = v; }
        constexpr value(int32_t v) : type_(type::Int32), value_() { value_.int32 = v; }
        constexpr value(int64_t v) : type_(type::Int64), value_() { value_.int64 = v; }

        constexpr value(float v) : type_(type::Float), value_() { value_.f = v; }
        constexpr value(double v) : type_(type::Double), value_() { value_.d = v; }

        constexpr value(type::type_class t, uint8_t v) : type_(t), value_() { value_.uint8 = v; }

        value(const Value& v) :type_(type::None), value_() {
            switch(v.type_case()) {
            case Value::TYPE_NOT_SET: break;
            case Value::kNone: break;
            case Value::kEn: type_ = type::Enum; value_.uint8 = v.en(); break;
            case Value::kB: type_ = type::Bool; value_.b = v.b(); break;

            case Value::kU8: type_ = type::Uint8; value_.uint8 = (uint8_t) v.u8(); break;
            case Value::kU16: type_ = type::Uint16; value_.uint16 = (uint16_t) v.u16(); break;
            case Value::kU32: type_ = type::Uint32; value_.uint32 = v.u32(); break;
            case Value::kU64: type_ = type::Uint64; value_.uint64 = v.u32(); break;

            case Value::kI8: type_ = type::Int8; value_.int8 = (int8_t) v.i8(); break;
            case Value::kI16: type_ = type::Int16; value_.int16 = (int16_t) v.i16(); break;
            case Value::kI32: type_ = type::Int32; value_.int32 = v.i32(); break;
            case Value::kI64: type_ = type::Int64; value_.int64 = v.i32(); break;
            case Value::kF: type_ = type::Float; value_.f = v.f(); break;
            case Value::kD: type_ = type::Double; value_.d = v.d(); break;
            }
        }

        constexpr type::type_class get_type_class() const { return type_; }
        constexpr const box& get_box() const { return value_; }

        constexpr bool is_valid() { return type_ != type::Invalid; }

        template<typename T>
            T get() const {
                return unwrap<T>(*this);
            }

        void pack(Value* v) {
            switch(type_) {
            case type::Invalid: v->mutable_none(); break;
            case type::None: v->mutable_none(); break;
            case type::Enum: v->set_en(value_.uint8); break;
            case type::Bool: v->set_b(value_.b); break;
            case type::Uint8: v->set_u8(value_.uint8); break;
            case type::Uint16: v->set_u16(value_.uint16); break;
            case type::Uint32: v->set_u32(value_.uint32); break;
            case type::Uint64: v->set_u64(value_.uint64); break;
            case type::Int8: v->set_i8(value_.int8); break;
            case type::Int16: v->set_i16(value_.int16); break;
            case type::Int32: v->set_i32(value_.int32); break;
            case type::Int64: v->set_i64(value_.int64); break;
            case type::Float: v->set_f(value_.f); break;
            case type::Double: v->set_d(value_.d); break;
            }
        }
    private:
        type::type_class type_;
        box value_;
    };

    template<>
        constexpr bool unwrap<bool>(const value& v) {
            return v.get_box().b;
        }
    template<>
        constexpr uint8_t unwrap<uint8_t>(const value& v) {
            return v.get_box().uint8;
        }
    template<>
        constexpr uint16_t unwrap<uint16_t>(const value& v) {
            return v.get_box().uint16;
        }
    template<>
        constexpr uint32_t unwrap<uint32_t>(const value& v) {
            return v.get_box().uint32;
        }
    template<>
        constexpr uint64_t unwrap<uint64_t>(const value& v) {
            return v.get_box().uint64;
        }
    template<>
        constexpr int8_t unwrap<int8_t>(const value& v) {
            return v.get_box().int8;
        }
    template<>
        constexpr int16_t unwrap<int16_t>(const value& v) {
            return v.get_box().int16;
        }
    template<>
        constexpr int32_t unwrap<int32_t>(const value& v) {
            return v.get_box().int32;
        }
    template<>
        constexpr int64_t unwrap<int64_t>(const value& v) {
            return v.get_box().int64;
        }
    template<>
        constexpr float unwrap<float>(const value& v) {
            return v.get_box().f;
        }
    template<>
        constexpr double unwrap<double>(const value& v) {
            return v.get_box().d;
        }

    inline std::ostream& operator<<(std::ostream& o, const value& v) {
        switch (v.get_type_class()) {
        case type::Invalid: o << "invalid"; break;
        case type::None: o << "none"; break;
        case type::Bool: o << (v.get<bool>() ? "true" : "false"); break;
        case type::Enum: o << "enum(" << (int) v.get<uint8_t>() << ")"; break;
        case type::Uint8: o << (int) v.get<uint8_t>(); break;
        case type::Uint16: o << v.get<uint16_t>(); break;
        case type::Uint32: o << v.get<uint32_t>(); break;
        case type::Uint64: o << v.get<uint64_t>(); break;
        case type::Int8: o << (int) v.get<int8_t>(); break;
        case type::Int16: o << v.get<int16_t>(); break;
        case type::Int32: o << v.get<int32_t>(); break;
        case type::Int64: o << v.get<int64_t>(); break;
        case type::Float: o << v.get<float>(); break;
        case type::Double: o << v.get<double>(); break;
        }
        return o;
    }
}
#endif
