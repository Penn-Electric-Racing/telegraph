#ifndef __WIRE_INTERFACE_HPP__
#define __WIRE_INTERFACE_HPP__

#include "promise.hpp"
#include "value.hpp"
#include "coroutine.hpp"

#include <memory>
#include <cstdint>

namespace wire {
    class variable_base;
    class action_base;
    class subscription;
    using subscription_ptr = std::unique_ptr<subscription>;
    using interval = uint16_t;

    class source {
    public:
        virtual promise<subscription_ptr> subscribe(variable_base* v,  
                        interval min_interval, interval max_interval, interval timeout) = 0;
        virtual promise<value> call(action_base* a, value arg, interval timeout) = 0;
    };


}

#endif
