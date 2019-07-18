#include "variable.hpp"

#include "../errors.hpp"

#include <iomanip>

namespace per {
    variable::variable(const std::string& name, const type t) :
                            node(name), src_(nullptr), type_(t) {}

    variable::subscription*
    variable::subscribe(uint64_t rate) {
        if (!src_) return nullptr;
        return src_->subscribe(this, rate);
    }

    void
    variable::print(std::ostream& o, int ident) const {
        o << std::setfill(' ') << std::setw(ident) << "";
        o << type_.to_str() << ' ' << get_name() << std::endl;
    }
}
