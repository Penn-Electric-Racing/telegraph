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
    public:
        // both relay and connection must outlive
        // the remote_namespace object
        remote_namespace(relay* r, connection* conn);

        // sets up the uuid, handlers on the
        // connection
        bool init(io::yield_ctx& ctx);

        query_ptr<mount_info> mounts(io::yield_ctx& yield,
                                    const uuid& srcs_of=uuid(),
                                    const uuid& tgts_of=uuid()) const override;

        query_ptr<context_ptr> contexts(io::yield_ctx&, const uuid& by_uuid=uuid(), 
                                            const std::string& by_name=std::string(), 
                                            const std::string& by_type=std::string()) const override;

        query_ptr<task_ptr> tasks(io::yield_ctx&,  const uuid& by_uuid=uuid(), 
                                    const std::string& by_name=std::string(), 
                                    const std::string& by_type=std::string()) const override;

        std::shared_ptr<node> fetch(io::yield_ctx& yield, const uuid& uuid, 
                                    context_ptr owner=context_ptr()) const override;

        subscription_ptr subscribe(io::yield_ctx& yield,
               const uuid& ctx, const std::vector<std::string>& path,
               interval min_interval, interval max_interval, interval timeout) override;

        value call(io::yield_ctx& yield, const uuid& ctx, 
                const std::vector<std::string>& path, value arg, interval timeout) override;

        std::unique_ptr<data_query> query_data(io::yield_ctx& yield,
                const uuid& ctx, const std::vector<std::string>& path) const override;

        bool write_data(io::yield_ctx& yield, const uuid& ctx, 
                    const std::vector<std::string>& path,
                    const std::vector<data_point>& data) override;

        void mount(io::yield_ctx& yield, const uuid& src, const uuid& tgt) override;
        void unmount(io::yield_ctx& yield, const uuid& src, const uuid& tgt) override;
    private:
        relay* relay_;
        connection* conn_;
    };

    /*
    class remote_context : public context {
    public:
        remote_context(const std::string& name, const std::string& type, const info& i,
                         const std::shared_ptr<node>& tree);
        void reg(remote_namespace* ns);
        bool destroy(io::yield_ctx& yield) override;

        std::shared_ptr<node> fetch(io::yield_ctx&) override;

        query_ptr<mount_info> mounts(io::yield_ctx& yield,
                bool srcs=true, bool tgts=true) const override;

        bool mount(io::yield_ctx&, const context_ptr& src) override;
        bool unmount(io::yield_ctx&, const context_ptr& src) override;
    private:
        remote_namspace* ns_;
    };*/
}

#endif
