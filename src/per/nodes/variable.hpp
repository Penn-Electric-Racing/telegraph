#ifndef __PER_VARIABLE_HPP__
#define __PER_VARIABLE_HPP__

#include "node.hpp"
#include "../value.hpp"

#include <functional>

namespace per {
    class variable : public node {
    public:
        class subscription {
        public:
            virtual void notify(const std::function<void(value)>& func) = 0;
            // after unsubscribe is called, this subscription object
            // will probably be deleted and should be considered an invalid pointer
            virtual void unsubscribe() = 0;
        };
        // a variable source manages subscriptions
        // note that one source might service multiple variables
        class source {
        public:
            virtual subscription* subscribe(variable* v, uint64_t interval) = 0;
        };

        variable();
        variable(const std::string& name, const type t);

        inline void set_type(const type& t) { type_ = t; }
        constexpr const type& get_type() const { return type_; }

        // set the subscription manager
        inline void set_source(source* s) { src_ = s; }

        // for getting access to the update stream you have to subscribe
        // if this fails it will do so quietly 
        // and return a null subscriber object
        //
        // on subscribe it is expected that 
        // the data source will resend the
        // last varaible state
        //
        // the interval specifies the largest allowable interval between
        // state updates for debouncing values. A value of 0 indicates a
        // realtime subscription 
        // (i.e a message will immediately be sent to the CAN bus/forwarded over perdos
        //  everytime the state changes)
        //
        // the underlying data source will handle how to keep subscriptions alive across
        // multiple links the end user does not need to worry about that
        //
        // when you are done with the subscriber, you *must* call unsubscribe() or you
        // will cause a memory leak where subscribers are never cleaned up
        // and subscriptions are kept alive indefinitely
        subscription* subscribe(uint64_t interval=0);

        void print(std::ostream& o, int ident) const override;
    private:
        source* src_;
        type type_;
    };
}

#endif
