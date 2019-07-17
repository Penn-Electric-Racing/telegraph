#include "dummy_source.hpp"

namespace per {
    dummy_source::dummy_source() : subscribers_() {}

    void
    dummy_source::unsubscribe(subscriber* s) {
        vars_[s->var_].erase(s);
        subscribers_.erase(s);
        delete s;
    }

    variable::subscriber*
    dummy_source::subscribe(variable* v, uint64_t interval) {
        subscriber* s = new subscriber(interval, v, this);
        subscribers_.insert(s);
        vars_[v].insert(s);
        return s;
    }

    void
    dummy_source::subscriber::notify(const std::function<void(value)>& func) {
        funcs_.push_back(func);
    }

    void
    dummy_source::subscriber::unsubscribe() {
        src_->unsubscribe(this);
    }
}
