#ifndef __TELEGRAPH_COMMON_ADAPTER_HPP__
#define __TELEGRAPH_COMMON_ADAPTER_HPP__

#include <deque>
#include <functional>
#include <unordered_set>
#include <memory>

#include "data.hpp"

#include "../utils/io.hpp"
#include <boost/asio/error.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace telegraph {
    // adapters and variables are subscription providers
    class adapter_base {
    public:
        virtual ~adapter_base() {}
        // return null on failure
        virtual subscription_ptr subscribe(io::yield_ctx& yield, 
                float debounce, float refresh, float timeout) = 0;
        virtual void update(value v) = 0;
    };

    template<typename PollFunc, typename ChangeFunc, typename CancelFunc>
        class adapter : public adapter_base,
                        public std::enable_shared_from_this<adapter<PollFunc, ChangeFunc, CancelFunc>> {
        private:
            friend class sub;
            using wadapter_ptr = std::weak_ptr<adapter<PollFunc, ChangeFunc, CancelFunc>>;
            class sub : public subscription {
                friend class adapter;
            private:
                wadapter_ptr adapter_;
                time_point last_update_;
            public:
                sub(const wadapter_ptr& a, 
                   value_type t, float debounce, float refresh) 
                    : subscription(t, debounce, refresh),
                      adapter_(a), last_update_() {}

                // on destruct do immediate cancel
                ~sub() {
                    cancel();
                }

                void poll() override {
                    auto a = adapter_.lock();
                    if (!a) {
                        cancel();
                        return;
                    }
                    // reset the last update time
                    // so everything goes through
                    last_update_ = time_point();
                    a->poll();
                }

                void change(io::yield_ctx& yield, 
                        float debounce, float refresh, 
                        float timeout) override {
                    debounce_ = debounce;
                    refresh_ = refresh;
                    auto a = adapter_.lock();
                    if (!a) {
                        cancel();
                        return;
                    }
                    a->change(yield, timeout);
                }

                void cancel(io::yield_ctx& yield, float timeout) override {
                    if (!cancelled_) {
                        cancelled_ = true;
                        auto a = adapter_.lock();
                        if (!a) {
                            cancelled();
                            return;
                        }
                        a->cancel(yield, this, timeout);
                        cancelled();
                    }
                }

                void cancel() override {
                    if (!cancelled_) {
                        cancelled_ = true;
                        auto a = adapter_.lock();
                        if (a) {
                            a->cancel(this);
                        }
                        cancelled();
                    }
                }
            private:
                void update(time_point tp, value v) {
                    // check if enough time has expired to send another update
                    // for this sub or if last_update_ is at epoch (for poll())
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(tp - last_update_);
                    if (duration.count() > refresh_*1000 ||
                            last_update_.time_since_epoch().count() == 0) {
                        last_update_ = tp;
                        data(v);
                    }
                }
            };


            io::io_context& ioc_;
            value_type type_; // type of the variable

            // current values
            bool subscribed_;
            float debounce_;
            float refresh_;

            // if an op is running
            bool running_op_;
            std::deque<io::deadline_timer*> waiting_ops_;
            std::unordered_set<sub*> subs_;

            PollFunc poll_;
            ChangeFunc change_;
            CancelFunc cancel_;
        public:
            adapter(io::io_context& ioc, value_type t, 
                    PollFunc poll, ChangeFunc change, CancelFunc cancel) :
                    ioc_(ioc), type_(t), subscribed_(false),
                    debounce_(0), refresh_(0), 
                    running_op_(false), waiting_ops_(), subs_(),
                    poll_(poll), change_(change), cancel_(cancel) {}

            // will push out an update...
            void update(value v) override {
                // push out values...
                auto tp = std::chrono::system_clock::now();
                for (sub* s : subs_) s->update(tp, v);
            }

            // will block until the change subscribe
            // request goes through
            subscription_ptr subscribe(io::yield_ctx& yield, 
                    float min_interval, float max_interval, 
                    float timeout) override {
                // calculate new min_interval_
                auto wp = std::enable_shared_from_this<
                            adapter<PollFunc, ChangeFunc, CancelFunc>>::
                                weak_from_this();
                sub* s = new sub(wp,
                    type_, min_interval, max_interval);
                subs_.insert(s);
                if (!change(yield, timeout)) {
                    // create a new subscription object
                    subs_.erase(s);
                    return nullptr;
                }
                return std::unique_ptr<subscription>(s);
            }
        private:
            void poll() {
                poll_();
            }
            bool change(io::yield_ctx& yield, float timeout) {
                float new_db = std::numeric_limits<float>::infinity();
                float new_rf = std::numeric_limits<float>::infinity();
                // compute new min_intervals
                for (sub* s : subs_) {
                    new_db = std::min(new_db, s->get_debounce());
                    new_rf = std::min(new_rf, s->get_refresh());
                }
                // if we don't need a new subscription
                if (subscribed_ && new_db == debounce_ && 
                        new_rf == refresh_) {
                    return true;
                }
                // queue a request
                if (running_op_) {
                    // put in queue
                    io::deadline_timer qt{ioc_};
                    waiting_ops_.push_back(&qt);
                    boost::system::error_code ec;
                    qt.async_wait(yield.ctx[ec]);
                    if (ec != io::error::operation_aborted) {
                        waiting_ops_.erase(std::remove(waiting_ops_.begin(), 
                                            waiting_ops_.end(), &qt), 
                                                waiting_ops_.end());
                        // if we timed out!
                        return false;
                    } else {
                        // remove from front!
                        waiting_ops_.pop_front();
                    }
                }
                running_op_ = true;

                bool s = change_(yield, new_db, new_rf, timeout);

                running_op_ = false;
                // notify next person
                if (waiting_ops_.size() > 0) {
                    waiting_ops_.front()->cancel();
                }

                return s;
            }

            bool cancel(io::yield_ctx& yield, sub* s, float timeout) {
                if (running_op_) {
                    // put in queue
                    io::deadline_timer qt{ioc_};
                    waiting_ops_.push_back(&qt);
                    boost::system::error_code ec;
                    qt.async_wait(yield.ctx[ec]);
                    if (ec != io::error::operation_aborted) {
                        waiting_ops_.erase(std::remove(waiting_ops_.begin(), 
                                            waiting_ops_.end(), &qt), 
                                                waiting_ops_.end());
                        // if we timed out!
                        subs_.erase(s);
                        return false;
                    } else {
                        // remove from front!
                        waiting_ops_.pop_front();
                    }
                }
                running_op_ = true;
                subs_.erase(s);
                bool success = true;
                if (subs_.size() > 0) {
                    float new_db = std::numeric_limits<float>::infinity();
                    float new_rf = std::numeric_limits<float>::infinity();
                    // compute new min_intervals
                    for (sub* s : subs_) {
                        new_db = std::min(new_db, s->get_debounce());
                        new_rf = std::min(new_rf, s->get_refresh());
                    }
                    // if we don't need a new subscription
                    if (!subscribed_ || new_db != debounce_ ||
                            new_rf != refresh_) {
                        success = change_(yield, new_db, new_rf, timeout);
                    }
                } else {
                    success = cancel_(yield, timeout);
                }
                running_op_ = false;
                // notify next person
                if (waiting_ops_.size() > 0) {
                    waiting_ops_.front()->cancel();
                }
                return success;
            }

            // cancel immediately
            void cancel(sub* s) {
                auto sp = std::enable_shared_from_this<
                            adapter<PollFunc, ChangeFunc, CancelFunc>>::
                                shared_from_this();
                subs_.erase(s);
                io::spawn(ioc_, [sp, s] (io::yield_context yield) {
                    io::yield_ctx y{yield};
                    sp->cancel(y, s, 0.1); // 0.1 second timeout on cancel request
                });
            }
        };
}

#endif
