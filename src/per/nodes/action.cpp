#include "action.hpp"

#include "../errors.hpp"

#include <iomanip>

namespace per {
    action::action() : node(), arg_(), ret_(), executor_() {}
    action::action(const std::string& name, const type& arg, const type& ret) :
                node(name), arg_(arg), ret_(ret), executor_() {}

    value
    action::execute(const value& v) {
        if (!executor_) throw missing_error("Action is missing an executor to handle request");
        return value();
    }

    void
    action::print(std::ostream& o, int ident) const {
        o << std::setfill(' ') << std::setw(ident) << "";
        o << arg_.to_str() << " -> " << ret_.to_str() << ' ' << get_name() << std::endl;
    }
}
