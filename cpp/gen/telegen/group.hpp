#ifndef __TELEGEN_GROUP_HPP__
#define __TELEGEN_GROUP_HPP__

#include "node.hpp"
#include <initializer_list>
#include <array>

namespace telegen {
    template<size_t num_children>
        class group : public node {
        public:
            constexpr group(int32_t id, const char* name, const char* pretty, const char* desc,
                            const std::array<node*, num_children>& children) : 
                                node(id, name, pretty, desc), children_(children) {}

            void pack(telegraph_proto_Variable* v, int32_t parent) const {
            }

            void pack(telegraph_proto_Node* n, int32_t parent) const override {
            }
        private:
            std::array<node*, num_children> children_;
        };
}

#endif
