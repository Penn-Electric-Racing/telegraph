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
            io::deadline_timer refresh_timer_;
            time_point last_update_;
            value last_value_;

            void reset_refresh_timer() {
                refresh_timer_.cancel();
                if (refresh_ != subscription::DISABLED) {
                    refresh_timer_.expires_from_now(
                        boost::posix_time::milliseconds(
                            std::max(1, (int) (1000*refresh_))));
                    refresh_timer_.async_wait([this](const boost::system::error_code& ec) {
                        resend(ec);
                    });
                }
            }
        public:
            sub(io::io_context& ioc, const std::weak_ptr<publisher> pub,
                value_type t, float debounce, float refresh)
                    : subscription(t, debounce, refresh),
                        publisher_(pub),
                        refresh_timer_(ioc), last_update_(),
                        last_value_(value::none()) {}
            ~sub() {
                cancel();
            }
            void poll() {
                auto p = publisher_.lock();
                if (!p) return;
                last_value_ = value::invalid();
                update(std::chrono::system_clock::now(), p->value_);
            }
            void change(io::yield_ctx& yield,
                        float debounce, float refresh,
                        float timeout) override {
                debounce_ = debounce;
                refresh_ = refresh;
                reset_refresh_timer();
            }
            void cancel(io::yield_ctx& yield, 
                        float timeout) override {
                cancel();
            }
            void cancel() override {
                refresh_timer_.cancel();
                if (!cancelled_) {
                    cancelled_ = true;
                    // remove from publisher
                    auto p = publisher_.lock();
                    if(p) {
                        p->subs_.erase(this);
                    }
                    refresh_timer_.cancel();
                }
            }
        private:
            void resend(const boost::system::error_code& ec) {
                if (ec != boost::asio::error::operation_aborted &&
                        last_value_.is_valid()) {
                    auto p = publisher_.lock();
                    if (p) {
                        data(p->value_);
                    }
                }
            }

            void update(time_point tp, value v) {
                auto d = std::chrono::duration_cast<
                    std::chrono::milliseconds>(tp - last_update_);
                if (d.count() > 1000*debounce_ || !last_value_.is_valid()) {
                    last_update_ = tp;
                    last_value_ = v;
                    reset_refresh_timer();
                    data(v);
                }
            }
        };

        std::unordered_map<sub*, std::weak_ptr<sub>> subs_;
        io::io_context& ioc_;
        value_type type_;
        value value_;
    public:
        publisher(io::io_context& ioc, value_type t) : subs_(), ioc_(ioc), type_(t) {}
        ~publisher() {
            // copy since cancel() will remove from subs_
            std::unordered_map<sub*, std::weak_ptr<sub>> subs = subs_;
            for (auto w : subs) {
                auto s = w.second.lock();
                if (s) s->cancel();
            }
        }

        subscription_ptr subscribe(float min_interval, float max_interval) {
            auto s = std::make_shared<sub>(ioc_, weak_from_this(), 
                            type_, min_interval, max_interval);
            subs_.emplace(s.get(), s);
            return s;
        }

        void update(value v) {
            value_ = v;
            auto tp = std::chrono::system_clock::now();
            for (auto ws : subs_) {
                auto s = ws.second.lock();
                if (s) s->update(tp, v);
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