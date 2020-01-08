#include "namespace.hpp"
#include "../common/nodes.hpp"

namespace telegraph {
    local_namespace::local_namespace() 
        : namespace_(rand_uuid()) {}

    std::unique_ptr<query<mount_info>>
    local_namespace::mounts(boost::asio::yield_context yield,
            const uuid& srcs_of, const uuid& tgts_of) {
        return nullptr;
    }

    std::unique_ptr<query<context_ptr>>
    local_namespace::contexts(boost::asio::yield_context yield,
            const uuid& by_uuid, const std::string& by_name, 
            const std::string& by_type) {
        return nullptr;
    }

    std::unique_ptr<query<task_ptr>>
    local_namespace::tasks(boost::asio::yield_context yield,
            const uuid& by_uuid, const std::string& by_name,
            const std::string& by_type) {
        return nullptr;
    }

    std::shared_ptr<node>
    local_namespace::fetch(boost::asio::yield_context yield,
                        const uuid& uuid, context_ptr owner) {
        return nullptr;
    }

    subscription_ptr
    local_namespace::subscribe(boost::asio::yield_context yield,
            const uuid& ctx, const std::vector<std::string>& path,
            int32_t min_interval, int32_t max_interval) {
        return nullptr;
    }

    value
    local_namespace::call(boost::asio::yield_context yield,
            const uuid& ctx, const std::vector<std::string>& path, const value& arg) {
        return value();
    }

    std::unique_ptr<data_query>
    local_namespace::data(boost::asio::yield_context yield,
            const uuid& ctx, const std::vector<std::string>& path) {
        return nullptr;
    }

    bool
    local_namespace::write_data(boost::asio::yield_context yield,
            const uuid& ctx, const std::vector<std::string>& path,
            const std::vector<data_point>& data) {
        return false;
    }

    bool
    local_namespace::mount(boost::asio::yield_context yield,
            const uuid& src, const uuid& tgt) {
        return false;
    }

    bool
    local_namespace::unmount(boost::asio::yield_context yield,
            const uuid& src, const uuid& tgt) {
        return false;
    }

    void
    local_namespace::register_ctx(const std::shared_ptr<local_context>& ctx) {
        contexts_[ctx->get_uuid()] = ctx;
    }

    void
    local_namespace::unregister_ctx(const local_context& ctx) {
        contexts_.erase(ctx.get_uuid());
    }
}
