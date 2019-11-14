#ifndef __TELEGRAPH_DATAPOINT_HPP__
#define __TELEGRAPH_DATAPOINT_HPP__

#include <cinttypes>
#include "value.hpp"

namespace telegraph {
    class datapoint {
    public:
        datapoint(uint64_t time, value val) : time_(time), val_(val) {}
        constexpr uint64_t get_time() const { return time_; }
        constexpr const value& get_value() const { return val_; }
    private:
        uint64_t time_;
        value val_;
    };
}

#endif
