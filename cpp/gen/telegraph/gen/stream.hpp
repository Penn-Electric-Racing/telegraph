#ifndef __TELEGRAPH_GEN_STREAM_HPP__
#define __TELEGRAPH_GEN_STREAM_HPP__

#include "node.hpp"

namespace telegraph::gen {
    class stream : public node {
    public:
        constexpr stream(int32_t id) : node(id) {}
    };
};

#endif
