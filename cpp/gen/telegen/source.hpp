#ifndef __TELEGEN_INTERFACE_HPP__
#define __TELEGEN_INTERFACE_HPP__

#include "promise.hpp"
#include "value.hpp"
#include "coroutine.hpp"

#include <memory>
#include <cstdint>

namespace telegen {
    class variable_base;
    class action_base;
    class subscription;
    using subscription_ptr = std::unique_ptr<subscription>;

    class source {
    public:
        virtual promise<subscription_ptr> subscribe(variable_base* v,  
                        int32_t min_interval, int32_t max_interval) = 0;
        virtual promise<value> call(action_base* a, const value& arg) = 0;
    };


}

#endif
