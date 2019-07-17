#ifndef __PER_VARIABLE_HPP__
#define __PER_VARIABLE_HPP__

#include "node.hpp"
#include "../value.hpp"

#include <functional>

namespace per {
    class variable : public node {
    public:
        class subscriber {
        public:
            virtual void notify(const std::function<void(value)>& func) = 0;
            // after unsubscribe is called, this subscription object
            // will probably be deleted and should be considered an invalid pointer
            virtual void unsubscribe() = 0;
        };
        // a variable source manages subscriptions
        class source {
        public:
            virtual subscriber* subscribe(variable* v, uint64_t rate) = 0;
        };

        variable();
        variable(const std::string& name, const type t);

        inline void set_type(const type& t) { type_ = t; }
        constexpr const type& get_type() const { return type_; }

        // set the subscription manager
        inline void set_source(source* s) { src_ = s; }

        // for getting access to the update stream you have to subscribe
        // if this fails it will return a null subscriber
        subscriber* subscribe(uint64_t rate=0);

        void print(std::ostream& o, int ident) const override;
    private:
        source* src_;
        type type_;
    };
}

#endif
