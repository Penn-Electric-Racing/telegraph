#ifndef __TELEGRAPH_DATAPOINT_HPP__
#define __TELEGRAPH_DATAPOINT_HPP__

#include "value.hpp"
#include "../utils/signal.hpp"
#include "../utils/io_fwd.hpp"

#include <cinttypes>
#include <memory>

namespace telegraph {
    using interval = uint16_t;

    class subscription {
    public:

        inline subscription(interval min_interval, interval max_interval) 
            : on_data(), on_cancel(), closed_(false),
            min_interval_(min_interval), max_interval_(max_interval) {}

        /**
         * On destruction cancel() will automatically be called if is_cancelled() is false
         */
        virtual ~subscription() {}

        constexpr interval get_min_interval() const { return min_interval_; }
        constexpr interval get_max_interval() const { return max_interval_; }

        /**
         * Whether this subscription is getting data
         */
        constexpr bool is_cancelled() const { return closed_; }

        /**
         * Returns true if the rate change was successful,
         * returns false if the operation timed out (failed).
         *
         * If the operation timed out, this subscription will be cancelled.
         */
        virtual bool change(io::yield_ctx&, interval min_interval, interval max_interval, 
                                interval timeout=1000) = 0;

        /**
         * Returns true if the board responded.
         * Returns false if the board did not respond
         * by the timeout (if the timeout is zero, this means no timeout)
         *
         * Either way is_cancelled() should true immediately after this call
         * is made (not when it completes).
         */
        inline virtual bool cancel(io::yield_ctx& yield, interval timeout=1000) = 0;

        signal<io::yield_ctx&, value> on_data;
        signal<io::yield_ctx&> on_cancel;
    protected:
        bool closed_;
        interval min_interval_;
        interval max_interval_;
    };
    using subscription_ptr = std::unique_ptr<subscription>;

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
