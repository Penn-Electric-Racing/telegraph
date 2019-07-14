#ifndef __PER_VARIABLE_HPP__
#define __PER_VARIABLE_HPP__

#include "node.hpp"
#include "../value.hpp"
#include "../utils/signal.hpp"

namespace per {
    // note that the variable node
    // does not store it's last known state
    // this is intentionally out of the scope of the variable
    // class and is meant to be handled by a data manager in a 
    // context
    class variable : public node {
    public:
        variable();
        variable(const std::string& name, const value::type t);

        constexpr const value::type& get_type() const { return type_; }

        void write(const value& v); // will trigger the write request signals
        void update(const value& v); // will trigger an update notification

        signal<value> on_write; // write requests come across here
        signal<value> on_update; // update notifications
    private:
        value::type type_;
    };
}

#endif
