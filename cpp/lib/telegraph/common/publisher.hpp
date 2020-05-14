#ifndef __TELEGRAPH_COMMON_PUBLISHER_HPP__
#define __TELEGRAPH_COMMON_PUBLISHER_HPP__

#include <memory>
#include <unordered_set>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/error.hpp>

#include "data.hpp"

namespace telegraph {
    class publisher : public std::enable_shared_from_this<publisher> {
    private:
        class sub : public subscription {
            friend class publisher;
        private:
            std::weak_ptr<publisher> publisher_;
            io::deadline_timer max_timer_;
            time_point last_update_;
            value current_;
        public:
            sub(io::io_context& ioc, const std::weak_ptr<publisher> pub,
                value_type t, float min_interval, float max_inteval)
                    : subscription(t, min_interval, max_inteval),
                        publisher_(pub),
                        max_timer_(ioc), last_update_(),
                        current_(value::invalid()) {}
            ~sub() {
                cancel();
            }
            void change(io::yield_ctx& yield,
                        float min_interval, float max_interval,
                        float timeout) override {
                min_interval_ = min_interval_;
                max_interval_ = max_interval_;
                max_timer_.cancel();
                if (max_interval_ != subscription::NO_RESEND) {
                    max_timer_.expires_from_now(
                        boost::posix_time::milliseconds(
                            std::max(1, (int) (1000*max_interval_))));
                    max_timer_.async_wait([this](const boost::system::error_code& ec) {
                        resend(ec);
                    });
                }
            }
            void cancel(io::yield_ctx& yield, 
                        float timeout) override {
                cancel();
            }
            void cancel() override {
                max_timer_.cancel();
                if (!cancelled_) {
                    cancelled_ = true;
                    // remove from publisher
                    auto p = publisher_.lock();
                    p->subs_.erase(this);
                    max_timer_.cancel();
                }
            }
        private:
            void resend(const boost::system::error_code& ec) {
                if (ec != boost::asio::error::operation_aborted &&
                        current_.is_valid()) {
                    data(current_);
                }
            }

            void update(time_point tp, value v) {
                current_ = v;
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(tp - last_update_);
                if (!current_.is_valid() || duration.count() > min_interval_*1000) {
                    last_update_ = tp;
                    if (max_interval_ != subscription::NO_RESEND) {
                        max_timer_.cancel();
                        max_timer_.expires_from_now(
                            boost::posix_time::milliseconds(
                                std::max(1, (int) (1000*max_interval_))));
                        max_timer_.async_wait([this](const boost::system::error_code& ec) {
                            resend(ec);
                        });
                    }
                    data(v);
                }
            }
        };

        std::unordered_set<sub*> subs_;
        io::io_context& ioc_;
        value_type type_;
    public:
        publisher(io::io_context& ioc, value_type t) : subs_(), ioc_(ioc), type_(t) {}

        subscription_ptr subscribe(float min_interval, float max_interval) {
            sub* s = new sub(ioc_, weak_from_this(), 
                            type_, min_interval, max_interval);
            subs_.insert(s);
            return std::unique_ptr<sub>(s);
        }

        void update(value v) {
            auto tp = std::chrono::system_clock::now();
            for (sub* s : subs_) {
                s->update(tp, v);
            }
        }
        publisher& operator<<(value v) {
            update(v);
            return *this;
        }
    };
    using publisher_ptr = std::shared_ptr<publisher>;
}

#endif