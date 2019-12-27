#ifndef __TELEGEN_INTERFACE_HPP__
#define __TELEGEN_INTERFACE_HPP__

namespace telegen {
    class generic_variable;
    class generic_subscription;

    class interface {
    public:
        virtual generic_subscription subscribe(generic_variable* v, 
                    int32_t min_interval, int32_t max_interval) = 0;
    };
}

#endif
