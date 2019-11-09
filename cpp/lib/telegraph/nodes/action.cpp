#include "action.hpp"

#include "../errors.hpp"

#include <iomanip>

namespace telegraph {
    action::action(const std::string& name, const std::string& pretty,
                        const std::string& desc, const type& arg, const type& ret) :
                node(name, pretty, desc), arg_(arg), ret_(ret), executor_() {}

    void
    action::execute(const value& v, const std::function<void(value)> cb) {
        if (!executor_) throw missing_error("Action is missing an executor to handle request");
        executor_->exec(this, v, cb);
    }

    void
    action::print(std::ostream& o, int ident) const {
        o << std::setfill(' ') << std::setw(ident) << "";
        o << arg_.to_str() << " -> " << ret_.to_str() << ' ' << get_name();
    }
}
