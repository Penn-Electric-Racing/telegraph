#include "context.hpp"
#include "namespace.hpp"

namespace telegraph {
    local_context::local_context(const std::string& name, 
            const std::string& type, const info& i) 
        : context(rand_uuid(), name, type, i), tree_(nullptr), ns_(nullptr) {
    }

    std::shared_ptr<node>
    local_context::fetch(io::yield_context yield) {
        return tree_;
    }

    void
    local_context::reg(local_namespace* ns) {
        if (ns_) throw "cannot register context in two namespaces!";
        ns_ = ns;
        ns_->register_ctx(std::static_pointer_cast<local_context>(shared_from_this()));
    }

    std::unique_ptr<query<mount_info>>
    local_context::mounts(io::yield_context ctx, bool srcs, bool tgts) const {
        return nullptr;
    }

    bool
    local_context::destroy(io::yield_context ctx) {
        return false;
    }
}
