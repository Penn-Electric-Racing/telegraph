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

    // FIXME: this is an ugly associated function. If/when `Arbitrary Self Types` lands in stable
    // Rust, we should rewrite this as a method. The reason we accept an Rc<RefCell<Self>> is
    // because we need to hand a weak pointer to the subscription.
    pub fn subscribe(
        publisher: Rc<RefCell<Self>>,
        refresh: Duration,
        debounce: Duration,
    ) -> Rc<RefCell<Subscription>> {
        let sub = Rc::new(RefCell::new(Subscription::new(
            refresh,
            debounce,
            Rc::downgrade(&publisher),
        )));

        publisher
            .borrow_mut()
            .subscriptions
            .push(Rc::downgrade(&sub));

        sub
    }

    pub fn update(&mut self, v: Value) {
        self.value = Some(v);
        let now = Instant::now();
        for weak_sub in &self.subscriptions {
            if let Some(sub) = weak_sub.upgrade() {
                sub.borrow_mut().update(now, v);
            }
        }
    }
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
    last_value: Option<Value>,
    last_update: Option<Instant>,
    refresh: Duration,
    debounce: Duration,
    // TODO: what how are we going to implement the refresh timer?
    // io::deadline_timer refresh_timer_;
    cancelled: bool,
}

impl Subscription {
    fn new(refresh: Duration, debounce: Duration, publisher: Weak<RefCell<Publisher>>) -> Self {
        Self {
            publisher,
            last_value: None,
            last_update: None,
            refresh,
            debounce,
            cancelled: false,
        }
    }

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

    fn reset_refresh_timer(&mut self) {
        //  refresh_timer_.cancel();
        if !self.cancelled {
            // refresh_timer_.expires_from_now(
            //     boost::posix_time::milliseconds(
            //         std::max(1, (int) (1000*refresh_))));
            // refresh_timer_.async_wait([this](const boost::system::error_code& ec) {
            //     resend(ec);
            // });
        }
    }

    pub fn poll(&mut self) {
        if let Some(publisher) = self.publisher.upgrade() {
            if let Some(value) = publisher.borrow().value {
                self.last_value = None;
                self.update(Instant::now(), value);
            }
        }
    }

    pub fn change(&mut self, debounce: Duration, refresh: Duration) {
        self.debounce = debounce;
        self.refresh = refresh;
        self.reset_refresh_timer();
    }


    fn update(&mut self, instant: Instant, value: Value) {
        if let Some(last_update) = self.last_update {
            // If we have already updated within the debounce window and we have some previous
            // value, no need to update again
            if instant - last_update < self.debounce && self.last_value.is_some() {
                return;
            }
        }

        self.last_update = Some(instant);
        self.last_value = Some(value);
        self.reset_refresh_timer();
        // TODO: what do we do here? It looks like `data` is a signal in the boost code
        // self.data(value);
    }

    // TODO, this should take some kind of error code
    fn resend(&mut self) {
        // TODO: why do we check if the last value is valid if we are just going to send the
        // publisher's value anyway?
        //  if (ec != boost::asio::error::operation_aborted && last_value_.is_valid()) {
        if self.last_value.is_some() {
            if let Some(_publisher) = self.publisher.upgrade() {
                // data(publisher.borrow().value)
            }
        }
        //  }
    }

}

impl Drop for Subscription {
    fn drop(&mut self) {
        self.cancel()
    }
}
