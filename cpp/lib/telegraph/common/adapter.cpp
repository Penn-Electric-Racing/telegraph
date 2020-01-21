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
            const std::function<void(interval, interval)>& change,
            const std::function<void()> cancel)
        : ioc_(ioc), change_sub_(change), cancel_sub_(cancel), 
          subscribed_(false), min_interval_(0), 
          max_interval_(0) {}


    subscription_ptr
    adapter::subscribe(io::yield_ctx& yield, 
            interval min_interval, interval max_interval, interval timeout) {
        sub* s = new sub(this, min_interval, max_interval);
        subs_.insert(s);

        // if we successfully changed the subscription
        if (recalculate(yield, timeout)) {
            return subscription_ptr(s);
        } else {
            subs_.erase(s);
            delete s;
            return subscription_ptr();
        }
    }

    bool
    adapter::change(io::yield_ctx& yield, interval new_min, interval new_max, interval timeout) {
        io::deadline_timer timer{ioc_, boost::posix_time::milliseconds(timeout)};
        if (current_op_ != nullptr) {
            io::deadline_timer wait{ioc_, boost::posix_time::milliseconds(timeout)};
            waiting_ops_.push_back(&wait);
            wait.async_wait(yield.ctx); // wait for our turn
            if (current_op_) { // turn never came!
                return false;
            }
        }

        // no operations in the queue!
        // we have the floor muhahahaha

        if (!subscribed_) {
            min_interval_ = std::numeric_limits<interval>::max();
            max_interval_ = 0;
        }

        // first determine if we need to re-request
        bool rerequest = false;
        if (subscribed_) {
            for (sub* s : subs_) {
                new_min = std::min(s->get_min_interval(), new_min);
                new_max = new_max > 0 ? std::min(s->get_max_interval(), new_max) : new_max;
            }
            if (new_min < min_interval_ || new_max < max_interval_ ||
                    (new_max > 0 && max_interval_ == 0)) {
                min_interval_ = std::min(new_min, min_interval_);
                max_interval_ = max_interval_ > 0 ? std::min(new_max, max_interval_) : new_max;
                rerequest = true;
            }
        } else {
            // not yet subscribed, always re-request
            min_interval_ = new_min;
            max_interval_ = new_max;
            rerequest = true;
        }

        if (rerequest) {
            // make the request!
            current_op_ = &timer;
            op_success_ = false;
            change_sub_(new_min, new_max);
            timer.async_wait(yield.ctx);

            subscribed_ = op_success_;

            current_op_ = nullptr;
            // notify the next person
            // that they can make their request
            if (waiting_ops_.size() > 0) {
                io::deadline_timer* t = waiting_ops_.front();
                waiting_ops_.pop_front(); // remove their timer
                t->cancel(); // cancel their timer so they can go
            }
        } 
        if (!subscribed_) {
            // if the request failed, we need
        }
        return subscribed_;
    }

    bool
    adapter::cancel(io::yield_ctx& yield, sub* s, interval timeout) {
        subs_.erase(s);
        io::deadline_timer timer{ioc_, boost::posix_time::milliseconds(timeout)};
        if (current_op_ != nullptr) {
            io::deadline_timer wait{ioc_, boost::posix_time::milliseconds(timeout)};
            waiting_ops_.push_back(&wait);
            wait.async_wait(yield.ctx); // wait for our turn
            if (current_op_) { // turn never came!
                return false;
            }
        }

        if (subs_.size() == 0) {
            current_op_ = &timer;
            op_success_ = false;
            cancel_sub_();
            timer.async_wait(yield.ctx);
            current_op_ = nullptr;
            // notify the next person
            // that they can make their request
            if (waiting_ops_.size() > 0) {
                io::deadline_timer* t = waiting_ops_.front();
                waiting_ops_.pop_front(); // remove their timer
                t->cancel(); // cancel their timer so they can go
            }
            return op_success_;
        } else {
            return change(yield, std::numeric_limits<interval>::max(), 0, timeout);
        }
    }
}
