#ifndef __PER_DATAPOINT_HPP__
#define __PER_DATAPOINT_HPP__

#include <cinttypes>
#include "value.hpp"

namespace per {
    class datapoint {
        uint64_t time;
        value val;
    };
}

#endif
