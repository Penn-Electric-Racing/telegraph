#include "variable.hpp"

#include "../errors.hpp"

namespace per {
    variable::variable(const std::string& name, const value::type t) :
                            node(name), on_write(), on_update(), type_(t) {}

    void
    variable::write(const value& v) {
        if (v.get_type() != type_) 
            throw bad_type_error("Used wrong type on variable write");
        on_write(v);
    }

    void
    variable::update(const value& v) {
        if (v.get_type() != type_) 
            throw bad_type_error("Used wrong type on variable write");
        on_update(v);
    }
}
