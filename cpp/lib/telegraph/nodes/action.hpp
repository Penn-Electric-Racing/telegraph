#ifndef __TELEGRAPH_ACTION_HPP__
#define __TELEGRAPH_ACTION_HPP__

#include "node.hpp"
#include "../value.hpp"
#include "../utils/signal.hpp"

#include <functional>
#include <future>

namespace telegraph {

    // an action represents a callable interface
    // where you can send a value
    // and get a response value
    // note that the value and return types can be "empty"
    // in which case this acts like a signal
    class action : public node {
    public:
        class executor {
        public:
            virtual value exec(const action* a, const value& v) = 0;
        };

        action(const std::string& name, const std::string& pretty, const std::string& desc,
                   const type& arg, const type& ret);

        constexpr const type& get_arg_type() const { return arg_; }
        constexpr const type& get_ret_type() const { return ret_; }

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
