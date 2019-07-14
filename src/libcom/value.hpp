#ifndef __PER_VALUE_HPP__
#define __PER_VALUE_HPP__

namespace per {

    // a value should be able to contain 
    // any value type, efficiently
    class value {
    public:
        enum type {
            INVALID, EMPTY,
            BOOL,
            UINT8, UINT16, UINT32, UINT64,
            INT8, INT16, INT32, INT64,
            FLOAT, DOUBLE
        };

        constexpr value() : type_(EMPTY), value_() {}
        constexpr value(bool b) : type_(BOOL), value_{ .b = b } {}
        constexpr value(uint8_t v) : type_(UINT8), value_{ .uint8  = v } {}
        constexpr value(uint16_t v) : type_(UINT16), value_{ .uint16  = v } {}
        constexpr value(uint32_t v) : type_(UINT32), value_{ .uint32  = v } {}
        constexpr value(uint64_t v) : type_(UINT64), value_{ .uint64  = v } {}

        constexpr value(int8_t v) : type_(INT8), value_{ .int8  = v } {}
        constexpr value(int16_t v) : type_(INT16), value_{ .int16  = v } {}
        constexpr value(int32_t v) : type_(INT32), value_{ .int32  = v } {}
        constexpr value(int64_t v) : type_(INT64), value_{ .int64  = v } {}

        constexpr value(float v) : type_(FLOAT), value_{ .f = v } {}
        constexpr value(double v) : type_(DOUBLE), value_{ .d = v } {}

        constexpr type get_type() const { return type_; }
    private:
        type type_;
        union {
            bool b;
            uint8_t uint8;
            uint16_t uint16;
            uint32_t uint32;
            uint64_t uint64;
            int8_t int8;
            int16_t int16;
            int32_t int32;
            int64_t int64;
            float f;
            double d;
        } value_;
    };
}
#endif
