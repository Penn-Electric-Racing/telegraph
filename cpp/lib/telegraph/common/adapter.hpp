#ifndef __TELEGRAPH_COMMON_ADAPTER_HPP__
#define __TELEGRAPH_COMMON_ADAPTER_HPP__

#include <deque>
#include <functional>
#include <unordered_set>

#include "data.hpp"
#include "../utils/io_fwd.hpp"

#include <boost/asio/deadline_timer.hpp>

namespace telegraph {


    /**
     * An adapter does de-multiplexing for a single stream of data.
     *
     * Note that the adapter must outlive any subscriptions that it creates
     */
    /*
    class adapter {
    private:
        class sub : public subscription {
            friend class adapter;
        public:
            inline sub(adapter* a, 
                    interval min_interval, 
                    interval max_interval) 
                : subscription(min_interval, max_interval),
                  adapter_(a), req_min_(min_interval), 
                  req_max_(max_interval) {}

            void change(io::yield_ctx& yield, 
                    interval min_interval, interval max_interval, 
                    interval timeout) override;

            void cancel(io::yield_ctx& yield, interval timeout) override;
            void cancel() override;
        private:
            void update(value v);
            void set_cancelled();

            adapter* adapter_;
            interval req_min_;
            interval req_max_;
        };
        std::function<bool(io::yield_ctx&, interval, interval, interval)> change_sub_;
        std::function<bool(io::yield_ctx&, interval)> cancel_sub_;
    public:
        adapter(io::io_context& ioc, 
                const std::function<bool(io::yield_ctx&, interval, interval, 
                                        interval timeout)>& change,
                const std::function<bool(io::yield_ctx&, interval timeout)> cancel,
                const std::function<void()> cancel_imm);

        // on update
        void update(value v);

        subscription_ptr subscribe(io::yield_ctx& yield, 
                interval min_interval, interval max_interval, 
                interval timeout=1000);
    private:
        void failure(io::yield_ctx&);
        bool change(io::yield_ctx&, interval new_min, 
                interval new_max, interval timeout);
        bool cancel(io::yield_ctx&, sub* s, interval timeout);
        void cancel(sub* s);

        io::io_context& ioc_;


        bool subscribed_;
        interval min_interval_;
        interval max_interval_;

        bool running_op_;
        std::deque<io::deadline_timer*> waiting_ops_;

        std::unordered_set<sub*> subs_;

        // we keep this around to send when a new subscriber connects
        value last_val_; 
    };
    */
}

#endif
