#ifndef __TELEGRAPH_NAMESPACE_HPP__
#define __TELEGRAPH_NAMESPACE_HPP__

#include "collection.hpp"
#include "value.hpp"
#include "data.hpp"
#include "params.hpp"

#include "../utils/uuid.hpp"
#include "../utils/io_fwd.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace telegraph {
    class namespace_;
    class variable;
    class action;
    class node;

    class subscription;
    using subscription_ptr = std::shared_ptr<subscription>;

    class datapoint;
    class data_query;
    using data_query_ptr = std::shared_ptr<data_query>;

    class context;
    using context_ptr = std::shared_ptr<context>;

    // underscore is to not conflict with builtin
    // namespace token
    class namespace_ {
    public:
        // public intentionally to make subclasses
        // go through this collection.
        collection_ptr<context_ptr> contexts;

        namespace_() {
            contexts = std::make_shared<collection<context_ptr>>();
        }

        virtual context_ptr create(io::yield_ctx&, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p) = 0;

        virtual void destroy(io::yield_ctx&, const uuid& u) = 0;
    };


    class context : public std::enable_shared_from_this<context> {
    public:
        inline context(io::io_context& ioc, 
                const uuid& uuid, const std::string_view& name, 
                const std::string_view& type, const params& p, bool headless) : 
                    ioc_(ioc), uuid_(uuid), headless_(headless), name_(name),
                    type_(type), params_(p) {}

        constexpr io::io_context& get_executor() { return ioc_; }

        virtual std::shared_ptr<namespace_> get_namespace() = 0;
        virtual std::shared_ptr<const namespace_> get_namespace() const = 0;

        const bool is_headless() const { return headless_; }
        const std::string& get_name() const { return name_; }
        const std::string& get_type() const { return type_; }
        const params& get_params() const { return params_; }
        const uuid& get_uuid() const { return uuid_; }

        virtual params_stream_ptr request(io::yield_ctx&, const params& p) = 0;

        virtual std::shared_ptr<node> fetch(io::yield_ctx& ctx) = 0;

        // tree manipulation functions
        virtual subscription_ptr  subscribe(io::yield_ctx& ctx, 
                                const std::vector<std::string_view>& variable,
                                float min_interval, float max_interval,
                                float timeout) = 0;
        virtual subscription_ptr  subscribe(io::yield_ctx& ctx, 
                                const variable* v,
                                float min_interval, float max_interval,
                                float timeout) = 0;

        virtual value call(io::yield_ctx& ctx, action* a, value v, float timeout) = 0;
        virtual value call(io::yield_ctx& ctx, const std::vector<std::string_view>& a, 
                                    value v, float timeout) = 0;

        virtual bool write_data(io::yield_ctx& yield, variable* v, 
                                    const std::vector<datapoint>& data) = 0;
        virtual bool write_data(io::yield_ctx& yield, const std::vector<std::string_view>& var,
                                    const std::vector<datapoint>& data) = 0;

        virtual data_query_ptr query_data(io::yield_ctx& yield, const variable* v) = 0;
        virtual data_query_ptr query_data(io::yield_ctx& yield, const std::vector<std::string_view>& v) = 0;

        virtual void destroy(io::yield_ctx& yield) = 0;
        signal<> destroyed;
    protected:
        io::io_context& ioc_;
        const uuid uuid_;
        const bool headless_;
        const std::string name_;
        const std::string type_;
        const params params_;
    };

    // query_key specialization
    // for context/info so that we can do get() on a query
    // to get by the uuid
    template<>
        struct collection_key<std::shared_ptr<context>> {
            typedef uuid type;
            static uuid get(const std::shared_ptr<context>& c) { 
                return c->get_uuid(); 
            }
        };
}

#endif
