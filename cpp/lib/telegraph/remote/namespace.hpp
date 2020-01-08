#ifndef __TELEGRAPH_REMOTE_NAMESPACE_HPP__
#define __TELEGRAPH_REMOTE_NAMESPACE_HPP__

#include "context.hpp"
#include "connection.hpp"

#include "../common/namespace.hpp"
#include "../utils/uuid.hpp"

#include <memory>

namespace telegraph {
    class relay;

    class remote_namespace : public namespace_ {
        friend class remote_context;
    public:
        // both relay and connection must outlive
        // the remote_namespace object
        remote_namespace(relay* r, connection* conn);

        // sets up the uuid, handlers on the
        // connection
        bool init(io::yield_context ctx);

        std::unique_ptr<query<mount_info>> mounts(
                boost::asio::yield_context,
                const uuid& srcs_of=uuid(),
                const uuid& tgts_of=uuid()) override;

        std::unique_ptr<query<context_ptr>> contexts(
                boost::asio::yield_context,
                const uuid& by_uuid=uuid(), const std::string& by_name=std::string(), 
                const std::string& by_type=std::string()) override;

        std::unique_ptr<query<task_ptr>> tasks(
                boost::asio::yield_context, const uuid& by_uuid=uuid(),
                const std::string& by_name=std::string(), 
                const std::string& by_type=std::string()) override;

        // returns a tree allocated on the heap
        // if context is supplied, the tree will 
        // be bound to that context for calls
        std::shared_ptr<node> fetch(
                boost::asio::yield_context yield, const uuid& uuid, 
                context_ptr owner=context_ptr()) override;

        subscription_ptr subscribe(
                boost::asio::yield_context yield,
               const uuid& ctx, const std::vector<std::string>& path,
               int32_t min_interval, int32_t max_interval) override;

        value call(boost::asio::yield_context yield,
                const uuid& ctx, const std::vector<std::string>& path,
                const value& arg) override;

        std::unique_ptr<data_query> data(
                boost::asio::yield_context yield,
                const uuid& ctx, const std::vector<std::string>& path) override;

        bool write_data(boost::asio::yield_context yield,
                    const uuid& ctx, const std::vector<std::string>& path,
                    const std::vector<data_point>& data) override;

        bool mount(boost::asio::yield_context yield, 
                    const uuid& src, const uuid& tgt) override;
        bool unmount(boost::asio::yield_context yield,
                    const uuid& src, const uuid& tgt) override;

    private:
        relay* relay_;
        connection* conn_;
    };
}

#endif
