#include "namespace.hpp"
#include "../common/nodes.hpp"
#include "../utils/errors.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace telegraph {
    local_namespace::local_namespace(io::io_context& ioc) 
            : namespace_(),
              ioc_(ioc), context_factories_() {}

    context_ptr 
    local_namespace::create(io::yield_ctx& yield, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p) {
        auto it = context_factories_.find(type);
        if (it == context_factories_.end()) return nullptr;
        else {
            auto c = (it->second)(yield, ioc_, name, type, p);
            if (c) c->reg(yield, shared_from_this());
            return c;
        }
    }

    void 
    local_namespace::destroy(io::yield_ctx& y, const uuid& u) {
        context_ptr c = contexts->get(u);
        if (!c) throw missing_error("No context with uuid " + boost::lexical_cast<std::string>(u));
        c->destroy(y);
    }


    local_context::local_context(io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type,
                const params& i, const std::shared_ptr<node>& tree, bool headless) : 
                context(ioc, rand_uuid(), name, type, i, headless), tree_(tree), ns_() {
        if (tree) {
            tree->set_owner(weak_from_this());
        }
    }

    void
    local_context::reg(io::yield_ctx& yield, const std::shared_ptr<local_namespace>& ns) {
        if (ns_.lock()) throw missing_error("already registered");
        if (!ns) throw missing_error("cannot register null namespace");
        ns_ = ns;
        ns->contexts->add_(shared_from_this());
    }

    void
    local_context::destroy(io::yield_ctx& yield) {
        auto s = ns_.lock();
        if (!s) return;
        s->contexts->remove_by_key_(get_uuid());
        ns_.reset();
        destroyed(yield);
    }

    local_component::local_component(io::io_context& ioc, const std::string_view& name, 
                                    const std::string_view& type, const params& i) :
                        local_context(ioc, name, type, i, nullptr, true) {}
}
