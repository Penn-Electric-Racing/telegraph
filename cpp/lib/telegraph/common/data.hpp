#ifndef __TELEGRAPH_DATAPOINT_HPP__
#define __TELEGRAPH_DATAPOINT_HPP__

#include "type.hpp"
#include "value.hpp"
#include "../utils/signal.hpp"
#include "../utils/io_fwd.hpp"

#include <cinttypes>
#include <memory>
#include <chrono>

namespace telegraph {
    class subscription {
    public:
        constexpr static float NO_RESEND = 0;
        subscription(value_type t, float min_interval, float max_interval) 
            : cancelled_(false), type_(t),
            min_interval_(min_interval), max_interval_(max_interval) {}

        /**
         * On destruction cancel() should be triggered
         */
        virtual ~subscription() {}

        constexpr const value_type& get_type() const { return type_; }
        constexpr float get_min_interval() const { return min_interval_; }
        constexpr float get_max_interval() const { return max_interval_; }

        /**
         * Whether this subscription is getting data
         */
        constexpr bool is_cancelled() const { return cancelled_; }

        virtual void change(io::yield_ctx&, float min_interval, 
                            float max_interval, float timeout) = 0;
        virtual void cancel(io::yield_ctx& yield, float timeout) = 0;
        virtual void cancel() = 0; // cancel immediately

        signal<value> data;
        signal<> cancelled;
    protected:
        bool cancelled_;
        value_type type_;
        float min_interval_;
        float max_interval_;
    };
    using subscription_ptr = std::unique_ptr<subscription>;

    using time_point = std::chrono::time_point<std::chrono::system_clock>;
    /**
     */
    class data_point {
    public:
        data_point(uint64_t time, value val) : time_(time), val_(val) {}
        constexpr uint64_t get_time() const { return time_; }
        constexpr const value& get_value() const { return val_; }
    private:
        uint16_t time_;
        value val_;
    };

    class data_query {
    };
    using data_query_ptr = std::unique_ptr<data_query>;
}

#endif
