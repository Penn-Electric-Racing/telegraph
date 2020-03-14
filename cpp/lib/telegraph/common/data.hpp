#ifndef __TELEGRAPH_DATAPOINT_HPP__
#define __TELEGRAPH_DATAPOINT_HPP__

#include "value.hpp"
#include "../utils/signal.hpp"
#include "../utils/io_fwd.hpp"
#include "../utils/info.hpp"

#include <cinttypes>
#include <memory>

namespace telegraph {
    using interval = uint16_t;

    class info_stream {
    public:
        info_stream() : data(), cancelled(), cancelled_(false) {}
        ~info_stream() { cancel(); }

        signal<io::yield_ctx&, const info&> data;
        signal<> cancelled;

        constexpr bool is_cancelled() const { return cancelled_; }

        void cancel() {
            if (!cancelled_) {
                cancelled_ = true;
                cancelled();
            }
        }
    protected:
        bool cancelled_;
    };

    using info_stream_ptr = std::unique_ptr<info_stream>;

    class subscription {
    public:

        subscription(interval min_interval, interval max_interval) 
            : cancelled_(false),
            min_interval_(min_interval), max_interval_(max_interval) {}

        /**
         * On destruction cancel() should automatically be called if is_cancelled() is false
         */
        virtual ~subscription() = 0;

        constexpr interval get_min_interval() const { return min_interval_; }
        constexpr interval get_max_interval() const { return max_interval_; }

        /**
         * Whether this subscription is getting data
         */
        constexpr bool is_cancelled() const { return cancelled_; }

        virtual void change(io::yield_ctx&, interval min_interval, interval max_interval, 
                                            interval timeout=1000) = 0;
        virtual void cancel(io::yield_ctx& yield, interval timeout=1000) = 0;

        /**
         * Cancel and don't wait for a response
         */
        virtual void cancel() = 0; 

        signal<value> data;
        signal<> cancelled;
    protected:
        bool cancelled_;
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
