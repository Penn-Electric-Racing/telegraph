#include "namespace.hpp"
#include "../common/nodes.hpp"

namespace telegraph {
    remote_namespace::remote_namespace(relay* r, connection* conn)
                        : namespace_(uuid()), relay_(r), conn_(conn) {
    }

    query_ptr<mount_info>
    remote_namespace::mounts(io::yield_ctx&,
            const uuid& srcs_of, const uuid& tgts_of) const {
        return nullptr;
    }

    query_ptr<context_ptr>
    remote_namespace::contexts(io::yield_ctx& yield,
            const uuid& by_uuid, const std::string& by_name, 
            const std::string& by_type) const {
        return nullptr;
    }

    query_ptr<task_ptr>
    remote_namespace::tasks(io::yield_ctx& yield,
            const uuid& by_uuid, const std::string& by_name,
            const std::string& by_type) const {
        return nullptr;
    }

    std::shared_ptr<node>
    remote_namespace::fetch(io::yield_ctx& yield,
                        const uuid& uuid, context_ptr owner) const {
        return nullptr;
    }

    subscription_ptr
    remote_namespace::subscribe(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path,
            int32_t min_interval, int32_t max_interval) {
        return nullptr;
    }

    value
    remote_namespace::call(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path, const value& arg) {
        return value();
    }

    std::unique_ptr<data_query>
    remote_namespace::data(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path) const {
        return nullptr;
    }

    bool
    remote_namespace::write_data(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path,
            const std::vector<data_point>& data) {
        return false;
    }

    bool
    remote_namespace::mount(io::yield_ctx& yield,
            const uuid& src, const uuid& tgt) {
        return false;
    }

    bool
    remote_namespace::unmount(io::yield_ctx& yield,
            const uuid& src, const uuid& tgt) {
        return false;
    }
}
