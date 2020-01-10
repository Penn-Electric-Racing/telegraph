#ifndef __TELEGEN_PUBLISHER_HPP__
#define __TELEGEN_PUBLISHER_HPP__

#include "nodes.hpp"
#include "source.hpp"

namespace telegen {
    class publisher_base : public source, public coroutine {
    };

    template<typename T, typename Clock>
        class publisher : public publisher_base {
            friend class sub_impl;
        public:
            class sub_impl : public subscription {
            public:
                sub_impl(int32_t min_interval, int32_t max_interval) : 
                    subscription(min_interval, max_interval),
                    last_time_(0), delayed_until_(0), next_resend_(0) {}

                ~sub_impl() {
                    if (!cancelled_) cancel();
                }

                promise<> cancel() override {
                    if (!cancelled_) {
                        pub_->subs_->remove(this);
                        cancelled_ = true;
                    }
                    // return accepting promise
                    return promise<>(promise_status::Resolved); 
                }

                promise<> change(int32_t min_int, int32_t max_int) override {
                    if (delayed_until_ > 0) {
                        delayed_until_ = last_time_ + min_int;
                    }
                    min_interval_ = min_int;
                    max_interval_ = max_int;
                }

                void push(const T& v, int32_t now_time) {
                    if (min_interval_ == 0 || 
                            last_time_ + min_interval_ > now_time) {
                        cb_(v);
                        last_time_ = now_time;
                    }
                    if (max_interval_) next_resend_ = now_time + max_interval_;
                }

                void push_late(const T& v) {
                    cb_(v);
                    delayed_until_ = 0;
                }

                void resend(const T& v) {
                    if (max_interval_) next_resend_ += max_interval_;
                    else next_resend_ = 0;
                }
            private:
                // the last time a value was sent out
                uint32_t last_time_;

                uint32_t delayed_until_; 
                uint32_t next_resend_;

                publisher<T, Clock>* pub_;
                bool cancelled_;
            };

            publisher(Clock& c) : subs_(), clock_(c) {}

            void update(const T& v) {
                last_val_ = v;
            }

            promise<subscription_ptr> subscribe(variable_base* v,
                        int32_t min_inteval, int32_t max_interval) {
                sub_impl* sub = new sub_impl(min_interval, max_interval);
                return promise<subscription_ptr>(promise_status::Rejected);
            }

            promise<value> call(action_base* a, const value& arg) {
                return promise<value>(promise_status::Rejected);
            }
        private:
            T last_val_;
            std::vector<sub_impl*> subs_;
            Clock& clock_;
        };

    template<typename F, typename Arg, typename Ret>
        class action_handler {
        public:
            action_handler(F& f) : func_(f) {}

            promise<subscription_ptr> subscribe(variable_base* v,
                        int32_t min_inteval, int32_t max_interval) {
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
