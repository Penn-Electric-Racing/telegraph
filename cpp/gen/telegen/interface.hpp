#ifndef __TELEGEN_INTERFACE_HPP__
#define __TELEGEN_INTERFACE_HPP__

#include <memory>
#include <cstdint>

namespace telegen {
    class variable_base;
    class action_base;
    class subscription;

    class interface {
    public:
        virtual std::unique_ptr<subscription> 
            subscribe(variable_base* v,  
                    int32_t min_interval, int32_t max_interval) = 0;
    };
}

#endif
