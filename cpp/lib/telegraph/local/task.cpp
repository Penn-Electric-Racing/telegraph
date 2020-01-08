#include "task.hpp"

namespace telegraph {
    local_task::local_task(const std::string& name, 
                           const std::string& type, const info& i)
        : task(rand_uuid(), name, type, i), ns_(nullptr) {
    }

    void
    local_task::reg(local_namespace* ns) {
        if (ns_) throw "cannot register a task in two namespaces!";
    }

    void
    local_task::destroy() {
        // TODO: call release_task on local_namespace
    }
}
