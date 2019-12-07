#ifndef __TELEGEN_STREAM_HPP__
#define __TELEGEN_STREAM_HPP__

#include "node.hpp"

namespace telegen {
    class stream : public node {
    public:
        constexpr stream(int32_t id, const char* name, const char* pretty,
                                const char* desc) : node(id, name, pretty, desc) {}
    };
};

#endif
