#ifndef __TELEGRAPH_REMOTE_NAMESPACE_HPP__
#define __TELEGRAPH_REMOTE_NAMESPACE_HPP__

#include "context.hpp"

#include "../common/namespace.hpp"
#include "../utils/uuid.hpp"

#include <memory>

namespace telegraph {
    class relay;
    class connection;

    class remote_namespace : public namespace_ {
        friend class remote_context;
    private:
        io::io_context& ioc_;
        connection& conn_;
        std::weak_ptr<remote_namespace> wptr_;
    public:
        // must also wait on init()!
        remote_namespace(io::io_context& ioc, connection& conn);
        // closes any open quieries on our side
        ~remote_namespace();

        void connect(io::yield_ctx& yield, const std::weak_ptr<remote_namespace>& wptr);
        bool is_connected() const;

        connection& get_conn() { return conn_; }
        const connection& get_conn() const { return conn_; }

        context_ptr create_context(io::yield_ctx&, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p) override;

        void destroy_context(io::yield_ctx&, const uuid& u) override;

        component_ptr create_component(io::yield_ctx&, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p) override;

        void destroy_component(io::yield_ctx&, const uuid& u) override;
    };

    class remote_context : public context {
    public:
        friend class remote_namespace;

        remote_context(io::io_context& ioc, 
                const std::shared_ptr<remote_namespace>& ns, const uuid& uuid,
                const std::string_view& name, const std::string_view& type, 
                const params& p) : context(ioc, uuid, name, type, p), ns_(ns) {}

        std::shared_ptr<namespace_> get_namespace() override { return ns_; }
        std::shared_ptr<const namespace_> get_namespace() const override { return ns_; }

        std::shared_ptr<node> fetch(io::yield_ctx& ctx) override;

        subscription_ptr  subscribe(io::yield_ctx& ctx, 
                                const std::vector<std::string_view>& variable,
                                float min_interval, float max_interval,
                                float timeout) override;

        value call(io::yield_ctx& ctx, const std::vector<std::string_view>& a, 
                            value v, float timeout) override;

        bool write_data(io::yield_ctx& yield, const std::vector<std::string_view>& var,
                                    const std::vector<data_point>& data) override;

        data_query_ptr query_data(io::yield_ctx& yield, const std::vector<std::string_view>& n) override;

        // mount-querying functions
        collection_ptr<mount_info> mounts(bool srcs=true, bool tgts=true) const override;

        void mount(io::yield_ctx& ctx, const context_ptr& src) override;
        void unmount(io::yield_ctx& ctx, const context_ptr& src) override;

        void destroy(io::yield_ctx& yield) override;
    private:
        // shared ptr since
        // remote_context depends on remote_namespace
        // (other way around for local_context and local_namespace)
        std::shared_ptr<remote_namespace> ns_;
    };
}

#endif
