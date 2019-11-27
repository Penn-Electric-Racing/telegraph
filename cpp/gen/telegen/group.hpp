#ifndef __TELEGEN_GROUP_HPP__
#define __TELEGEN_GROUP_HPP__

#include "node.hpp"
#include <array>

namespace telegen {
    template<size_t num_children>
        class group : public node {
        public:
            constexpr group(int32_t id, const char* name, const char* pretty, const char* desc,
                            const std::array<node*, num_children>& children) : 
                                node(id, name, pretty, desc), children_(children) {}
        private:
            std::array<node*, num_children> children_;
        };
}

#endif
