#ifndef __TELEGRAM_GEN_ACTION_HPP__
#define __TELEGRAM_GEN_ACTION_HPP__

#include "node.hpp"

namespace telegraph::gen {
    template<typename Arg, typename Ret>
        class action : public node {
        public:
            constexpr action(int32_t id) : node(id) {}

            Ret call(const Arg& a) {
                // do nothing for now
            }
        };
}

#endif
