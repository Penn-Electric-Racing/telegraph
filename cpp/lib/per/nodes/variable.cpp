#include "variable.hpp"

#include "../errors.hpp"

#include <iomanip>

namespace per {
    variable::variable(const std::string& name, const std::string& pretty, const std::string& desc,
                        const type t) : node(name, pretty, desc), type_(t) {}

    std::shared_ptr<variable::subscription>
    variable::subscribe(uint64_t min_interval, uint64_t max_interval) {
        std::shared_ptr<subscription> s = std::make_shared<subscription>(min_interval, max_interval);
        subscription* w = s.get();

        // if this gets called w will be valid because
        // before w is deleted on_cancel will be called to remove this handler
        on_dispose.add(s.get(), [w]() { w->cancel(); });

        s->on_cancel.add([this, w]() { on_dispose.remove(w); });

        on_subscribe(s);
        return s;
    }

    void
    variable::print(std::ostream& o, int ident) const {
        o << std::setfill(' ') << std::setw(ident) << "";
        o << type_.to_str() << ' ' << get_name() << std::endl;
    }
}
