#ifndef __PER_ACTION_HPP__
#define __PER_ACTION_HPP__

#include "node.hpp"
#include "../value.hpp"
#include "../utils/signal.hpp"

#include <functional>
#include <future>

namespace per {

    // an action represents a callable interface
    // where you can send a value 
    // and get a response value
    // note that the value can be "empty"
    // in which case this acts like a signal
    class action : public node {
    public:
        class executor {
        public:
            virtual value exec(const action* a, const value& v) = 0;
        };

        action();
        action(const std::string& name, 
           const type& arg, const type& ret);

        constexpr void set_executor(executor* ex) { executor_ = ex; }

        value execute(const value&v);

        void print(std::ostream& o, int ident) const override;
    private:
        type arg_;
        type ret_;
        executor* executor_;
    };
}

#endif
