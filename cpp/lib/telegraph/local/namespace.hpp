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
    class local_component;
    using local_context_ptr = std::shared_ptr<local_context>;
    using local_component_ptr = std::shared_ptr<local_component>;

    class local_namespace : 
            public std::enable_shared_from_this<local_namespace>,
            public namespace_ {
        friend class local_context;
    private:
        using context_factory = std::function<local_context_ptr(io::yield_ctx&,io::io_context&, 
                                    const std::string_view&, const std::string_view&, 
                                    const params&)>;

        io::io_context& ioc_;
        std::map<std::string, context_factory, std::less<>> context_factories_;
    public:
        local_namespace(io::io_context& ioc);

        void register_factory(const std::string& type, const context_factory& f) {
            context_factories_.emplace(std::make_pair(type, f));
        }

        context_ptr create(io::yield_ctx& yield, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p) override;

        void destroy(io::yield_ctx& y, const uuid& u) override;
    };

    class local_context : public context {
    public:
        local_context(io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type,
                const params& i, const std::shared_ptr<node>& tree, bool headless=false);

        std::shared_ptr<namespace_> get_namespace() override { return ns_.lock(); }
        std::shared_ptr<const namespace_> get_namespace() const override { return ns_.lock(); }

        void reg(io::yield_ctx& yield, const std::shared_ptr<local_namespace>& ns);
        void destroy(io::yield_ctx& yield) override;

        inline std::shared_ptr<node> fetch(io::yield_ctx&) override {  return tree_; }
    protected:
        std::shared_ptr<node> tree_;
        std::weak_ptr<local_namespace> ns_;
    };

    class local_component : public local_context {
    public:
        local_component(io::io_context& ioc, const std::string_view& name, 
                        const std::string_view&type, const params& i);

        std::shared_ptr<node> fetch(io::yield_ctx& ctx) override { return nullptr; }

        // tree manipulation functions
        subscription_ptr  subscribe(io::yield_ctx& ctx, 
                                const std::vector<std::string_view>& variable,
                                float min_interval, float max_interval,
                                float timeout) override { return nullptr; }
        subscription_ptr  subscribe(io::yield_ctx& ctx, 
                                const variable* v,
                                float min_interval, float max_interval,
                                float timeout) override { return nullptr; }
        value call(io::yield_ctx& ctx, action* a, value v, float timeout) override { return value::invalid(); }
        value call(io::yield_ctx& ctx, const std::vector<std::string_view>& a, 
                            value v, float timeout) override { return value::invalid(); }

        bool write_data(io::yield_ctx& yield, variable* v, 
                                    const std::vector<datapoint>& data) override { return false; }
        bool write_data(io::yield_ctx& yield, const std::vector<std::string_view>& var,
                                    const std::vector<datapoint>& data) override { return false; }

        data_query_ptr query_data(io::yield_ctx& yield, const variable* v) override { return nullptr; }
        data_query_ptr query_data(io::yield_ctx& yield, const std::vector<std::string_view>& v) override { return nullptr; }
    };
}

#endif
