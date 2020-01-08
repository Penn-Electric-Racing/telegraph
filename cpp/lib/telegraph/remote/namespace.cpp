#include "namespace.hpp"
#include "../common/nodes.hpp"

namespace telegraph {
    remote_namespace::remote_namespace(relay* r, connection* conn)
                        : namespace_(uuid()), relay_(r), conn_(conn) {
    }

    std::unique_ptr<query<mount_info>>
    remote_namespace::mounts(boost::asio::yield_context yield,
            const uuid& srcs_of, const uuid& tgts_of) {
        return nullptr;
    }

    std::unique_ptr<query<context_ptr>>
    remote_namespace::contexts(boost::asio::yield_context yield,
            const uuid& by_uuid, const std::string& by_name, 
            const std::string& by_type) {
        return nullptr;
    }

    std::unique_ptr<query<task_ptr>>
    remote_namespace::tasks(boost::asio::yield_context yield,
            const uuid& by_uuid, const std::string& by_name,
            const std::string& by_type) {
        return nullptr;
    }

    std::shared_ptr<node>
    remote_namespace::fetch(boost::asio::yield_context yield,
                        const uuid& uuid, context_ptr owner) {
        return nullptr;
    }

    subscription_ptr
    remote_namespace::subscribe(boost::asio::yield_context yield,
            const uuid& ctx, const std::vector<std::string>& path,
            int32_t min_interval, int32_t max_interval) {
        return nullptr;
    }

    value
    remote_namespace::call(boost::asio::yield_context yield,
            const uuid& ctx, const std::vector<std::string>& path, const value& arg) {
        return value();
    }

    std::unique_ptr<data_query>
    remote_namespace::data(boost::asio::yield_context yield,
            const uuid& ctx, const std::vector<std::string>& path) {
        return nullptr;
    }

    bool
    remote_namespace::write_data(boost::asio::yield_context yield,
            const uuid& ctx, const std::vector<std::string>& path,
            const std::vector<data_point>& data) {
        return false;
    }

    bool
    remote_namespace::mount(boost::asio::yield_context yield,
            const uuid& src, const uuid& tgt) {
        return false;
    }

    bool
    remote_namespace::unmount(boost::asio::yield_context yield,
            const uuid& src, const uuid& tgt) {
        return false;
    }
}
