#ifndef __TELEGRAPH_GEN_VARIABLE_HPP__
#define __TELEGRAPH_GEN_VARIABLE_HPP__

#include "node.hpp"

namespace telegraph::gen {
    template<typename T>
        class variable : public node {
        public:
            constexpr variable(int32_t id) : node(id) {}

            // write to this variable
            variable& operator<<(const T& t) {
                last_ = t;
            }
        protected:
            T last_;
        };
}

#endif
