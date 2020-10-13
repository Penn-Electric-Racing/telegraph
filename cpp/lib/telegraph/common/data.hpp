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
        static constexpr float DISABLED = std::numeric_limits<float>::infinity();

        subscription(value_type t, float debounce, float refresh) 
            : cancelled_(false), type_(t),
            debounce_(debounce), refresh_(refresh) {}

        /**
         * On destruction cancel() should be triggered
         */
        virtual ~subscription() {}

        constexpr const value_type& get_type() const { return type_; }
        constexpr float get_debounce() const { return debounce_; }
        constexpr float get_refresh() const { return refresh_; }

        /**
         * Whether this subscription is getting data
         */
        constexpr bool is_cancelled() const { return cancelled_; }

        // request a reset of the timers along the subscription path
        // and a re-transmission of the latest value
        virtual void poll() = 0;

        virtual void change(io::yield_ctx&, float debounce, 
                            float refresh, float timeout) = 0;
        virtual void cancel(io::yield_ctx& yield, float timeout) = 0;
        virtual void cancel() = 0; // cancel immediately

        signal<value> data;
        signal<> cancelled;
    protected:
        bool cancelled_;
        value_type type_;
        float debounce_;
        float refresh_;
    };
    using subscription_ptr = std::shared_ptr<subscription>;

    using time_point = std::chrono::time_point<std::chrono::system_clock>;
    /**
     */
    class datapoint {
    private:
        time_point time_;
        value val_;
    public:
        datapoint(time_point time, value val) : time_(time), val_(val) {}
        constexpr time_point get_time() const { return time_; }
        constexpr value get_value() const { return val_; }

        void pack(Datapoint* dp) {
            auto micro = 
                std::chrono::duration_cast<std::chrono::microseconds>(
                    time_.time_since_epoch());
            dp->set_timestamp(micro.count());
            val_.pack(dp->mutable_value());
        }

        static time_point now() {
            return std::chrono::system_clock::now();
        }
    };

    class data_query {
    public:
        virtual const std::vector<datapoint>& get_current() const = 0;
        signal<const std::vector<datapoint>&> data;
    };
    using data_query_ptr = std::shared_ptr<data_query>;
}

#endif