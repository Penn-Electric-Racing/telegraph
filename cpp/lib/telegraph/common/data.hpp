#ifndef __TELEGRAPH_DATAPOINT_HPP__
#define __TELEGRAPH_DATAPOINT_HPP__

#include "value.hpp"
#include "../utils/signal.hpp"
#include "../utils/io_fwd.hpp"

#include <cinttypes>
#include <memory>

namespace telegraph {
    class subscription {
    public:
        inline subscription(uint32_t min_interval, uint32_t max_interval) 
            : on_data(), on_cancel(), min_interval_(min_interval), 
              max_interval_(max_interval) {}
        virtual ~subscription() {}

        virtual bool is_cancelled() = 0;
        virtual void cancel(io::yield_ctx&) = 0;

        signal<value> on_data;
        signal<> on_cancel;
    private:
        uint32_t min_interval_;
        uint32_t max_interval_;
    };
    using subscription_ptr = std::unique_ptr<subscription>;

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

    using data_query_ptr = std::unique_ptr<data_query>;
}

#endif
