#ifndef __WIRE_PUBLISHER_HPP__
#define __WIRE_PUBLISHER_HPP__

#include "nodes.hpp"
#include "source.hpp"

namespace wire {
    class publisher_base : public source, public coroutine {
    };

    template<typename T, typename Clock>
        class publisher : public publisher_base {
            friend class sub_impl;
        public:
            class sub_impl : public subscription {
            public:
                // the last time a value was sent out
                uint32_t last_time_;
                // the two alarms
                uint32_t delay_alarm_;
                uint32_t resend_alarm_;
                publisher<T, Clock>* pub_;

                sub_impl(publisher<T,Clock>* pub, int32_t min_interval, int32_t max_interval) : 
                    subscription(min_interval, max_interval),
                    last_time_(0), delay_alarm_(std::numeric_limits<uint32_t>::max()), 
                                   resend_alarm_(std::numeric_limits<uint32_t>::max()),
                    pub_(pub) {}

                ~sub_impl() {
                    // cancel immediately
                    if (!is_cancelled()) cancel(0);
                }

                bool is_cancelled() override {
                    return pub_ == nullptr;
                }

                promise<> cancel(interval timeout) override {
                    if (!is_cancelled()) {
                        if (pub_) 
                            pub_->subs_.erase(std::remove(pub_->subs_.begin(), 
                                        pub_->subs_.end(), this), pub_->subs_.end());
                        pub_ = nullptr;
                        cancel_cb_();
                    }
                    // return accepting promise
                    return promise<>(promise_status::Resolved); 
                }

                promise<> change(interval min_int, interval max_int, 
                                interval timeout) override {
                    min_interval_ = min_int;
                    max_interval_ = max_int;

                    if (!pub_) return promise<>(promise_status::Rejected);

                    if (delay_alarm_ < std::numeric_limits<uint32_t>::max()) {
                        delay_alarm_ = last_time_ + min_int;
                    }
                    if (max_interval_ > 0 && last_time_ > 0) {
                        resend_alarm_ = last_time_ + max_interval_;
                    }
                    int32_t next_alarm = delay_alarm_;
                    if (next_alarm == 0) next_alarm = resend_alarm_;
                    if (next_alarm > 0) pub_->set_alarm(next_alarm);

                    return promise<>(promise_status::Resolved);
                }

                void push(const T& v, int32_t now_time) {
                    // deal (messily) with timestamp wraparound
                    // should happen around 1/mo but let's make sure
                    // all alarms don't stop working just in case
                    if (now_time < last_time_) last_time_ = now_time;

                    if (now_time > last_time_ + min_interval_) {
                        // we are larger than min_interval 
                        // since last update
                        last_time_ = now_time;

                        value val(get_type_class<T>());
                        val.set<T>(v);
                        cb_(val);
                    } else {
                        // set an alarm
                        delay_alarm_ = last_time_ + min_interval_;
                        pub_->set_alarm(delay_alarm_);
                    }

                    if (max_interval_) {
                        resend_alarm_ = now_time + max_interval_;
                        pub_->set_alarm(resend_alarm_);
                    }
                }

                void push_delayed(const T& v, uint32_t now_time) {
                    value val(get_type_class<T>());
                    val.set<T>(v);
                    cb_(val);

                    last_time_ = delay_alarm_;
                    delay_alarm_ = std::numeric_limits<uint32_t>::max();
                    resend_alarm_ = max_interval_ == 0 ? std::numeric_limits<uint32_t>::max() 
                                                       : last_time_ + max_interval_;
                }

                void push_resend(const T& v, int32_t now_time) {
                    value val(get_type_class<T>());
                    val.set<T>(v);
                    cb_(val);

                    last_time_ = resend_alarm_;
                    resend_alarm_ = max_interval_ == 0 ? std::numeric_limits<uint32_t>::max()
                                                       : last_time_ + max_interval_;
                }
            };

            publisher(Clock* c) : initialized_(false), last_val_(0), 
                    next_alarm_(std::numeric_limits<uint32_t>::max()),
                    subs_(), clock_(c) {}

            publisher(Clock* c, variable<T>* var) : 
                    initialized_(false), last_val_(0), 
                    next_alarm_(std::numeric_limits<uint32_t>::max()),
                    subs_(), clock_(c) {
                var->set_owner(this);
            }

            ~publisher() {
                // in case somebody has deleted the publisher but is
                // keeping the subscriptions around!
                // this should not be necessary, but just in case
                for (sub_impl* s : subs_) s->pub_ = nullptr;
            }

            // non-copyable so that the subscriptions always point
            // to the right publisher
            publisher(const publisher<T, Clock>& p) = delete;
            void operator=(const publisher<T, Clock>&p) = delete;

            void use_for(variable<T>* var) {
                var->set_source(this);
            }

            void operator<<(const T& v) {
                if (v == last_val_) return;
                uint32_t now = clock_->millis();
                last_val_ = v;
                initialized_ = true;
                for (sub_impl* s : subs_) s->push(v, now);
            }


            void set_alarm(uint32_t alarm) {
                next_alarm_ = std::min(alarm, next_alarm_);
            }

            void recalculate_next() {
                next_alarm_ = std::numeric_limits<uint32_t>::max();
                for (sub_impl* i : subs_) {
                    next_alarm_ = std::min(next_alarm_, i->delay_alarm_);
                    next_alarm_ = std::min(next_alarm_, i->resend_alarm_);
                }
            }

            void resume() override {
                uint32_t now = clock_->millis();
                if (next_alarm_ < now && initialized_) {
                    for (sub_impl* i : subs_) {
                        if (i->delay_alarm_ < now) i->push_delayed(last_val_, now);
                        if (i->resend_alarm_ < now) i->push_resend(last_val_, now);
                    }
                    recalculate_next();
                }
            }

            promise<subscription_ptr> subscribe(variable_base* v,
                        interval min_interval, interval max_interval, interval timeout) override {
                sub_impl* sub = new sub_impl(this, min_interval, max_interval);
                subs_.push_back(sub);
                // if we have a value, send it out now
                if (initialized_) {
                    uint32_t now = clock_->millis();
                    next_alarm_ = now;
                    sub->resend_alarm_ = now;
                }
                return promise<subscription_ptr>(std::unique_ptr<subscription>(sub));
            }

            promise<value> call(action_base* a, value arg, interval timeout) override {
                return promise<value>(promise_status::Rejected);
            }
        private:
            bool initialized_;
            T last_val_;
            uint32_t next_alarm_;
            std::vector<sub_impl*> subs_;
            Clock* clock_;
        };

    template<typename F, typename Arg, typename Ret>
        class action_handler {
        public:
            action_handler(F& f) : func_(f) {}

            promise<subscription_ptr> subscribe(variable_base* v,
                        int32_t min_interval, int32_t max_interval) {
                return promise<subscription_ptr>(promise_status::Rejected);
            }
            promise<value> call(action_base* a, const value& arg) {
                return promise<value>(value(func_(arg.get<Arg>())));
            }
        private:
            F& func_;
        };


    // TODO: Finish implementing the publisher
    // scheduler
    class publisher_scheduler : public coroutine {
    private:
        int32_t next_time;
    };
}

#endif
