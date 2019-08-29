#ifndef __PER_DATAPOINT_HPP__
#define __PER_DATAPOINT_HPP__

#include <cinttypes>
#include "value.hpp"

namespace per {
    class datapoint {
    public:
        constexpr uint64_t get_time() const { return time_; }
        constexpr const value& get_value() const { return val_; }
    private:
        uint64_t time_;
        value val_;
    };
}

#endif
