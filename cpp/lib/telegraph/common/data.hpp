#ifndef __TELEGRAPH_DATAPOINT_HPP__
#define __TELEGRAPH_DATAPOINT_HPP__

#include "value.hpp"

#include <cinttypes>
#include <memory>

namespace telegraph {
    class subscription {
    };
    using subscription_ptr = std::shared_ptr<subscription>;

    class data_point {
    public:
        data_point(uint64_t time, value val) : time_(time), val_(val) {}
        constexpr uint64_t get_time() const { return time_; }
        constexpr const value& get_value() const { return val_; }
    private:
        uint64_t time_;
        value val_;
    };

    class data_query {
    };
}

#endif
