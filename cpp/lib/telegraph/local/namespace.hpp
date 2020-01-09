#ifndef __TELEGRAPH_LOCAL_NAMESPACE_HPP__
#define __TELEGRAPH_LOCAL_NAMESPACE_HPP__

#include "context.hpp"

#include "../utils/uuid.hpp"
#include "../utils/errors.hpp"

#include "../common/namespace.hpp"

#include <memory>

namespace telegraph {
    class local_namespace : public namespace_ {
        friend class local_context;
        friend class local_task;

        // these are queries, not sets
        // since then we can just use chain()
        // to do the mounts, contexts, and tasks queries implementations
        // and if somebody wants everything, we can just return
        // a copy of the original query pointers
    private:
        query_ptr<context_ptr> contexts_;
        query_ptr<task_ptr> tasks_;
        query_ptr<mount_info> mounts_;
    public:
        local_namespace();

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
               int32_t min_interval, int32_t max_interval) override;

        value call(io::yield_ctx& yield, const uuid& ctx, 
                const std::vector<std::string>& path, const value& arg) override;

        std::unique_ptr<data_query> data(io::yield_ctx& yield,
                const uuid& ctx, const std::vector<std::string>& path) const override;

        bool write_data(io::yield_ctx& yield, const uuid& ctx, 
                    const std::vector<std::string>& path,
                    const std::vector<data_point>& data) override;

        bool mount(io::yield_ctx& yield, const uuid& src, const uuid& tgt) override;
        bool unmount(io::yield_ctx& yield, const uuid& src, const uuid& tgt) override;
    };

    class local_context : public context {
    public:
        local_context(const std::string& name, const std::string& type, const info& i,
                     const std::shared_ptr<node>& tree);
        void reg(io::yield_ctx& yield, local_namespace* ns);
        bool destroy(io::yield_ctx& yield) override;

        inline std::shared_ptr<node> fetch(io::yield_ctx&) override {  return tree_; }

        query_ptr<mount_info> mounts(io::yield_ctx& yield,
                bool srcs=true, bool tgts=true) const override;
        bool mount(io::yield_ctx&, const context_ptr& src) override;
        bool unmount(io::yield_ctx&, const context_ptr& src) override;
    protected:
        const std::shared_ptr<node> tree_;
        local_namespace* ns_;
    };

    class local_task : public task {
    public:
        local_task(const std::string& name,
                const std::string& type, const info& i);

        void reg(io::yield_ctx& ctx, local_namespace* ns);
        void destroy(io::yield_ctx& ctx) override;
    protected:
        local_namespace* ns_;
    };
}

#endif
