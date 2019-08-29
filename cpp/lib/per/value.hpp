#ifndef __PER_VALUE_HPP__
#define __PER_VALUE_HPP__

#include <string>
#include <cinttypes>
#include <ostream>

#include "type.hpp"

namespace per {
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

        inline value() : type_(type::NONE), value_() {}
        inline value(bool b) : type_(type::BOOL), value_{ .b = b } {}
        inline value(uint8_t v) : type_(type::UINT8), value_{ .uint8  = v } {}
        inline value(uint16_t v) : type_(type::UINT16), value_{ .uint16  = v } {}
        inline value(uint32_t v) : type_(type::UINT32), value_{ .uint32  = v } {}
        inline value(uint64_t v) : type_(type::UINT64), value_{ .uint64  = v } {}

        inline value(int8_t v) : type_(type::INT8), value_{ .int8  = v } {}
        inline value(int16_t v) : type_(type::INT16), value_{ .int16  = v } {}
        inline value(int32_t v) : type_(type::INT32), value_{ .int32  = v } {}
        inline value(int64_t v) : type_(type::INT64), value_{ .int64  = v } {}

        inline value(float v) : type_(type::FLOAT), value_{ .f = v } {}
        inline value(double v) : type_(type::DOUBLE), value_{ .d = v } {}

        inline value(type::type_class t, uint8_t v) : type_(t), value_{ .uint8 = v } {}

        constexpr type::type_class get_type_class() const { return type_; }
        constexpr const box& get_box() const { return value_; }

        template<typename T>
            T get() const {
                return unwrap<T>(*this);
            }
    private:
        type::type_class type_; // only the type class, 
                                // see the stream for the actual type
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

    std::ostream& operator<<(std::ostream& o, const value& v);
}
#endif
