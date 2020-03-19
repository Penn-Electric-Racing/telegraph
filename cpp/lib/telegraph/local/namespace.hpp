#ifndef __TELEGRAPH_LOCAL_NAMESPACE_HPP__
#define __TELEGRAPH_LOCAL_NAMESPACE_HPP__

#include "context.hpp"

#include "../utils/uuid.hpp"
#include "../utils/errors.hpp"
#include "../utils/io_fwd.hpp"

#include "../common/namespace.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <map>
#include <functional>

namespace telegraph {
    class local_context;
    class local_task;
    using local_context_ptr = std::shared_ptr<local_context>;
    using local_task_ptr = std::shared_ptr<local_task>;

    class local_namespace : 
            public std::enable_shared_from_this<local_namespace>,
            public namespace_ {
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

        using task_factory = std::function<local_task_ptr(io::yield_ctx&,io::io_context&, 
                                    const std::string_view&, const std::string_view&,
                                    const params&, sources_map&&)>;

        using context_factory = std::function<local_context_ptr(io::yield_ctx&,io::io_context&, 
                                    const std::string_view&, const std::string_view&, 
                                    const params&, sources_map&&)>;

        io::io_context& ioc_;

        std::map<std::string, task_factory, std::less<>> task_factories_;
        std::map<std::string, context_factory, std::less<>> context_factories_;
    public:
        local_namespace(io::io_context& ioc);

        // add factories
        void register_task_factory(const std::string& type, const task_factory& f) {
            task_factories_.emplace(std::make_pair(type, f));
        }

        void register_context_factory(const std::string& type, const context_factory& f) {
            context_factories_.emplace(std::make_pair(type, f));
        }

        //

        context_ptr create_context(io::yield_ctx& yield, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p, sources_uuid_map&& srcs) override;

        task_ptr create_task(io::yield_ctx& yield, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p, sources_uuid_map&& srcs) override;

        void destroy_context(io::yield_ctx& y, const uuid& u) override;
        void destroy_task(io::yield_ctx& y, const uuid& u) override;
        //

        query_ptr<mount_info> mounts(io::yield_ctx& yield,
                                    const uuid& srcs_of=uuid(),
                                    const uuid& tgts_of=uuid()) const override;

        query_ptr<context_ptr> contexts(io::yield_ctx&, const uuid& by_uuid=uuid(), 
                                            const std::string_view& by_name=std::string(), 
                                            const std::string_view& by_type=std::string()) const override;

        query_ptr<task_ptr> tasks(io::yield_ctx&,  const uuid& by_uuid=uuid(), 
                                    const std::string_view& by_name=std::string(), 
                                    const std::string_view& by_type=std::string()) const override;

        std::shared_ptr<node> fetch(io::yield_ctx& yield, const uuid& uuid, 
                                    context_ptr owner=context_ptr()) const override;

        subscription_ptr subscribe(io::yield_ctx& yield,
               const uuid& ctx, const std::vector<std::string_view>& path,
               float min_interval, float max_interval, float timeout) override;

        value call(io::yield_ctx& yield, const uuid& ctx, 
                const std::vector<std::string_view>& path, value arg, float timeout) override;

        std::unique_ptr<data_query> query_data(io::yield_ctx& yield,
                const uuid& ctx, const std::vector<std::string_view>& path) const override;

        bool write_data(io::yield_ctx& yield, const uuid& ctx, 
                    const std::vector<std::string_view>& path,
                    const std::vector<data_point>& data) override;

        void mount(io::yield_ctx& yield, const uuid& src, const uuid& tgt) override;
        void unmount(io::yield_ctx& yield, const uuid& src, const uuid& tgt) override;

        void start_task(io::yield_ctx& yield, const uuid& task) override;
        void stop_task(io::yield_ctx& yield, const uuid& task) override;
        params_stream_ptr query_task(io::yield_ctx& yield, const uuid& task, const params& i) override;
    };

    class local_context : public context {
    public:
        local_context(io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type, 
                const params& i, const std::shared_ptr<node>& tree);

        std::shared_ptr<namespace_> get_namespace() override { return ns_.lock(); }
        std::shared_ptr<const namespace_> get_namespace() const override { return ns_.lock(); }

        void reg(io::yield_ctx& yield, const std::shared_ptr<local_namespace>& ns);
        void destroy(io::yield_ctx& yield) override;

        inline std::shared_ptr<node> fetch(io::yield_ctx&) override {  return tree_; }

        query_ptr<mount_info> mounts(io::yield_ctx& yield,
                bool srcs=true, bool tgts=true) const override;
        void mount(io::yield_ctx&, const context_ptr& src) override;
        void unmount(io::yield_ctx&, const context_ptr& src) override;
    protected:
        std::shared_ptr<node> tree_;
        std::weak_ptr<local_namespace> ns_;
    };

    class local_task : public task {
    public:
        local_task(io::io_context& ioc, const std::string_view& name,
                const std::string_view& type, const params& i);

        std::shared_ptr<namespace_> get_namespace() override { return ns_.lock(); }
        std::shared_ptr<const namespace_> get_namespace() const override { return ns_.lock(); }

        void reg(io::yield_ctx& ctx, const std::shared_ptr<local_namespace>& ns);
        void destroy(io::yield_ctx& ctx) override;
    protected:
        std::weak_ptr<local_namespace> ns_;
    };
}

#endif
