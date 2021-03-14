use std::cell::RefCell;
use std::rc::{Rc, Weak};
use std::time::{Duration, Instant};

use crate::types::Type;
use crate::value::Value;

pub struct Publisher {
    subscriptions: Vec<Weak<RefCell<Subscription>>>,
    // TODO: is there a good reason why we have this? What is this ever used for / why can't we get
    // this information from the value itself?
    value_type: Type,
    value: Option<Value>,
}

impl Publisher {
    pub fn new(value_type: Type) -> Self {
        Self {
            subscriptions: vec![],
            value_type,
            value: None,
        }
    }

    pub fn remove(&mut self, subscription: &Subscription) {
        let pos = self.subscriptions.iter().position(|s| {
            if let Some(s) = s.upgrade() {
                std::ptr::eq(s.as_ref().as_ptr(), subscription)
            } else {
                false
            }
        });

        if let Some(pos) = pos {
            self.subscriptions.remove(pos);
        }
    }

    // TODO: how are the types going to work out on this?
    // pub fn subscribe(self: Weak<RefCell<Self>>, min_interval: f64, max_interval: f64) -> Rc<RefCell<Subscription>> {
    //     panic!("unimplemented!");
    // }

//         subscription_ptr subscribe(float min_interval, float max_interval) {
//             auto s = std::make_shared<sub>(ioc_, weak_from_this(),
//                             type_, min_interval, max_interval);
//             subs_.emplace(s.get(), s);
//             return s;
//         }

//         void update(value v) {
//             value_ = v;
//             auto tp = std::chrono::system_clock::now();
//             for (auto ws : subs_) {
//                 auto s = ws.second.lock();
//                 if (s) s->update(tp, v);
//             }
//         }
//         publisher& operator<<(value v) {
//             update(v);
//             return *this;
//         }
//     };
//     using publisher_ptr = std::shared_ptr<publisher>;
// }
}

impl Drop for Publisher {
    fn drop(&mut self) {
        for sub in &self.subscriptions {
            if let Some(sub) = sub.upgrade() {
                sub.borrow_mut().cancel_quietly()
            }
        }
    }
}

pub struct Subscription {
    publisher: Weak<RefCell<Publisher>>,
    last_value: Value,
    last_update: Instant,
    // TODO: what how are we going to implement the refresh timer?
    // io::deadline_timer refresh_timer_;
    cancelled: bool,
}

impl Subscription {
    // TODO: what is handled by this subscription superclass?
    // sub(io::io_context& ioc, const std::weak_ptr<publisher> pub,
    //     value_type t, float debounce, float refresh)
    //         : subscription(t, debounce, refresh),
    //             publisher_(pub),
    //             refresh_timer_(ioc), last_update_(),
    //             last_value_(value::none()) {}


    /// This internal cancel method is called by a publisher when it is being dropped, because if
    /// the publisher calls cancel while `Drop`ing we don't need to remove this from it.
    fn cancel_quietly(&mut self) {
        if !self.cancelled {
            self.cancelled = true;
        }
        // refresh_timer_.cancel();
    }

    pub fn cancel(&mut self) {
        if !self.cancelled {
            self.cancelled = true;

            // FIXME: this won't work whenever the publisher calls this (or even when we get this
            // reference through the publisher), because we can't mutably borrow the publisher
            // while the publisher is already borrowed to give us a reference to this. We might
            // need to be more clever about this if we start panicking, but for now this should be
            // fine.

            // This is what we do in the C++ backend:
            //             // copy since cancel() will remove from subs_
            //             std::unordered_map<sub*, std::weak_ptr<sub>> subs = subs_;
            // Maybe what we need is a separate RefCell for the subscriptions?

            if let Some(publisher) = self.publisher.upgrade() {
                publisher.borrow_mut().remove(self);
            }

        }
        // refresh_timer_.cancel();
    }

//             void reset_refresh_timer() {
//                 refresh_timer_.cancel();
//                 if (refresh_ != subscription::DISABLED) {
//                     refresh_timer_.expires_from_now(
//                         boost::posix_time::milliseconds(
//                             std::max(1, (int) (1000*refresh_))));
//                     refresh_timer_.async_wait([this](const boost::system::error_code& ec) {
//                         resend(ec);
//                     });
//                 }
//             }

//             void poll() {
//                 auto p = publisher_.lock();
//                 if (!p) return;
//                 last_value_ = value::invalid();
//                 update(std::chrono::system_clock::now(), p->value_);
//             }
//             void change(io::yield_ctx& yield,
//                         float debounce, float refresh,
//                         float timeout) override {
//                 debounce_ = debounce;
//                 refresh_ = refresh;
//                 reset_refresh_timer();
//             }
//         private:
//             void resend(const boost::system::error_code& ec) {
//                 if (ec != boost::asio::error::operation_aborted &&
//                         last_value_.is_valid()) {
//                     auto p = publisher_.lock();
//                     if (p) {
//                         data(p->value_);
//                     }
//                 }
//             }

//             void update(time_point tp, value v) {
//                 auto d = std::chrono::duration_cast<
//                     std::chrono::milliseconds>(tp - last_update_);
//                 if (d.count() > 1000*debounce_ || !last_value_.is_valid()) {
//                     last_update_ = tp;
//                     last_value_ = v;
//                     reset_refresh_timer();
//                     data(v);
//                 }
//             }
}

impl Drop for Subscription {
    fn drop(&mut self) {
        self.cancel()
    }
}

