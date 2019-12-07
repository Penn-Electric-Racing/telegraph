#ifndef __TELEGEN_INTERFACE_HPP__
#define __TELEGEN_INTERFACE_HPP__

namespace telegen {
    class generic_variable;
    class generic_subscription;

    class interface {
        virtual void subscribed(generic_variable* v, generic_subscription* s) = 0;
    };
}

#endif
