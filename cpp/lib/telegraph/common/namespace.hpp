#ifndef __TELEGRAPH_NAMESPACE_HPP__
#define __TELEGRAPH_NAMESPACE_HPP__

#include "context.hpp"
#include "task.hpp"
#include "data.hpp"
#include "../utils/uuid.hpp"
#include "../utils/io.hpp"

namespace telegraph {
    // underscore is to not conflict with builtin
    // namespace token
    class namespace_ {
    public:
        namespace_(const uuid& uuid) : uuid_(uuid) {}

        const uuid& get_uuid() const { return uuid_; }

        virtual std::unique_ptr<query<mount_info>>
            mounts(io::yield_context,
                const uuid& srcs_of=uuid(),
                const uuid& tgts_of=uuid()) = 0;

        virtual std::unique_ptr<query<context_ptr>>
            contexts(io::yield_context,
                    const uuid& by_uuid=uuid(), const std::string& by_name="", 
                    const std::string& by_type="") = 0;

        virtual std::unique_ptr<query<task_ptr>>
            tasks(io::yield_context, const uuid& by_uuid=uuid(),
                    const std::string& by_name="", const std::string& by_type="") = 0;

        // if context is supplied, the tree may
        // be bound to that context for calls
        // if it would not otherwise have an owning context
        virtual std::shared_ptr<node>
            fetch(io::yield_context, const uuid& uuid, 
                    context_ptr owner=context_ptr()) = 0;

        virtual subscription_ptr
            subscribe(io::yield_context,
                    const uuid& ctx, const std::vector<std::string>& path,
                    int32_t min_interval, int32_t max_interval) = 0;
        virtual value
            call(io::yield_context, 
                    const uuid& ctx, const std::vector<std::string>& path,
                    const value& val) = 0;

        virtual std::unique_ptr<data_query>
            data(io::yield_context,
                    const uuid& ctx, const std::vector<std::string>& path) = 0;
        virtual bool
            write_data(io::yield_context,
                    const uuid& ctx, const std::vector<std::string>& path,
                    const std::vector<data_point>& data) = 0;

        virtual bool mount(io::yield_context, 
                    const uuid& src, const uuid& tgt) = 0;
        virtual bool unmount(io::yield_context,
                    const uuid& src, const uuid& tgt) = 0;

        // TODO: Context creation and destruction
    private:
        boost::uuids::uuid uuid_;
    };
}

#endif
