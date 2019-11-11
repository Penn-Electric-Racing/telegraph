#ifndef __TELEGEN_STREAM_HPP__
#define __TELEGEN_STREAM_HPP__

#include "node.hpp"

namespace telegen {
    class stream : public node {
    public:
        constexpr stream(int32_t id) : node(id) {}
    };
};

#endif
