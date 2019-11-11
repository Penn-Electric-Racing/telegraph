#ifndef __TELEGEN_ACTION_HPP__
#define __TELEGEN_ACTION_HPP__

#include "node.hpp"

namespace telegen {
    template<typename Arg, typename Ret>
        class action : public node {
        public:
            constexpr action(int32_t id) : node(id) {}

            Ret call(const Arg& a) {
                // do nothing for now
            }
        };

    template<typename Ret>
        class action<void, Ret> : public node {
        public:
            constexpr action(int32_t id) : node(id) {}

            Ret call() {
                // do nothing for now
            }
        };
}

#endif
