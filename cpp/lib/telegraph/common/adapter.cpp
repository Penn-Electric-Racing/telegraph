#include "adapter.hpp"

#include "../utils/io.hpp"
#include <boost/asio/post.hpp>

namespace telegraph {
    static const uint32_t max_time = std::numeric_limits<interval>::max();

    bool
    adapter::sub::change(io::yield_ctx& yield, interval min_interval, 
                        interval max_interval, interval timeout) {
        if (closed_ || !adapter_) return false;
        // wait for change
        bool c = adapter_->change(yield, min_interval, max_interval, timeout);
        if (c) {
            min_interval_ = min_interval;
            max_interval_ = max_interval;
        }
        return c;
    }

    void
    adapter::failure(io::yield_ctx& yield) {
        // we got back a change_sub failure,
        // assume we have been unsubscribed!
    }

    bool
    adapter::sub::cancel(io::yield_ctx& yield, interval timeout) {
        if (closed_ || !adapter_) return false;
        bool cancelled = adapter_->cancel(yield, this, timeout);
        closed_ = true;
        on_cancel(yield);
        return cancelled;
    }

    adapter::adapter(io::io_context& ioc,
            const std::function<bool(io::yield_ctx&, interval, interval, interval)>& change,
            const std::function<bool(io::yield_ctx&, interval)> cancel)
        : ioc_(ioc), change_sub_(change), cancel_sub_(cancel), 
          subscribed_(false), min_interval_(0), 
          max_interval_(0), 
          running_op_(false), waiting_ops_(),
          subs_(), last_val_() {}

    subscription_ptr
    adapter::subscribe(io::yield_ctx& yield, 
            interval min_interval, interval max_interval, interval timeout) {
        sub* s = new sub(this, min_interval, max_interval);

        // if we successfully changed the subscription
        if (change(yield, min_interval, max_interval, timeout)) {
            subs_.insert(s);
            return subscription_ptr(s);
        } else {
            delete s;
            return subscription_ptr();
        }
    }

    bool
    adapter::change(io::yield_ctx& yield, interval new_min, interval new_max, interval timeout) {
        if (running_op_) {
            io::deadline_timer wait{ioc_};
            waiting_ops_.push_back(&wait);
            wait.async_wait(yield.ctx); // wait for our turn
        }

        // no operations in the queue!
        // we have the floor muhahahaha

        if (!subscribed_) {
            min_interval_ = std::numeric_limits<interval>::max();
            max_interval_ = 0;
        }

        // first determine if we need to re-request
        bool subscribe = false;
        if (subscribed_) {
            for (sub* s : subs_) {
                new_min = std::min(s->get_min_interval(), new_min);
                new_max = new_max > 0 ? std::min(s->get_max_interval(), new_max) : new_max;
            }
            if (new_min < min_interval_ || new_max < max_interval_ ||
                    (new_max > 0 && max_interval_ == 0)) {
                // we need to re-sub
                new_min = std::min(new_min, min_interval_);
                new_max = max_interval_ > 0 ? std::min(new_max, max_interval_) : new_max;
                subscribe = true;
            }
        } else {
            subscribe = true;
        }

        if (subscribe) {
            // make the request!
            running_op_ = true;
            bool success = change_sub_(yield, new_min, new_max, timeout);
            running_op_ = false;
            if (success) {
                subscribed_ = true;
                min_interval_ = new_min;
                max_interval_ = new_max;
            }

            // notify the next person
            // that they can make their request
            if (waiting_ops_.size() > 0) {
                io::deadline_timer* t = waiting_ops_.front();
                waiting_ops_.pop_front(); // remove their timer
                t->cancel(); // cancel their timer so they can go
            }
            return success;
        }
        return true;
    }

    bool
    adapter::cancel(io::yield_ctx& yield, sub* s, interval timeout) {
        if (running_op_) {
            io::deadline_timer wait{ioc_};
            waiting_ops_.push_back(&wait);
            wait.async_wait(yield.ctx); // wait for our turn
        }

        subs_.erase(s);
        delete s;

        if (subs_.size() == 0) {
            running_op_ = true;
            bool success = cancel_sub_(yield, timeout);
            running_op_ = false;

            // notify the next person
            // that they can make their request
            if (waiting_ops_.size() > 0) {
                io::deadline_timer* t = waiting_ops_.front();
                waiting_ops_.pop_front(); // remove their timer
                t->cancel(); // cancel their timer so they can go
            }
            return success;
        } else {
            return change(yield, std::numeric_limits<interval>::max(), 0, timeout);
        }
    }
}
