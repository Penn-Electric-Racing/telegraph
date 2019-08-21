#include "dummy_source.hpp"

namespace per {

    dummy_source::dummy_source() : subscribers_() {}

    void
    dummy_source::update(variable* var, const value& v) {
        for (auto& sub : vars_[var]) {
            sub->send(v);
        }
    }

    void
    dummy_source::unsubscribe(subscription* s) {
        vars_[s->var_].erase(s);
        subscribers_.erase(s);
        delete s;
    }

    variable::subscription*
    dummy_source::subscribe(variable* v, uint64_t interval) {
        subscription* s = new subscription(interval, v, this);
        subscribers_.insert(s);
        vars_[v].insert(s);
        return s;
    }

    void
    dummy_source::subscription::send(const value& v) {
        for (auto& f : funcs_) f(v);
    }

    void
    dummy_source::subscription::notify(const std::function<void(value)>& func) {
        funcs_.push_back(func);
    }

    void
    dummy_source::subscription::unsubscribe() {
        src_->unsubscribe(this);
    }
}
