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
    using subscription_ptr = std::unique_ptr<subscription>;

    class data_point;
    class data_query;
    using data_query_ptr = std::unique_ptr<data_query>;

    class task;
    using task_ptr = std::shared_ptr<task>;

    class context;
    using context_ptr = std::shared_ptr<context>;

    struct mount_info {
        constexpr mount_info() 
            : src(), tgt() {}
        constexpr mount_info(const uuid& src, const uuid& tgt) 
            : src(src), tgt(tgt) {}
        uuid src;
        uuid tgt;
    };

    using sources_map = std::unordered_map<std::string,
              std::variant<context_ptr, std::unique_ptr<node>>>;

    // like sources_map, but has uuids instead of context_ptrs
    using sources_uuid_map = std::unordered_map<std::string,
              std::variant<uuid, std::unique_ptr<node>>>;

    // underscore is to not conflict with builtin
    // namespace token
    class namespace_ {
    public:
        // these are public intentionally to force subclasses
        // to go through these variables!
        collection_ptr<mount_info> mounts;
        collection_ptr<context_ptr> contexts;
        collection_ptr<task_ptr> tasks;

        namespace_() {
            mounts = std::make_shared<collection<mount_info>>();
            contexts = std::make_shared<collection<context_ptr>>();
            tasks = std::make_shared<collection<task_ptr>>();
        }

        virtual context_ptr create_context(io::yield_ctx&, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p, sources_uuid_map&& srcs) = 0;

        virtual void destroy_context(io::yield_ctx&, const uuid& u) = 0;

        virtual task_ptr create_task(io::yield_ctx&, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p, sources_uuid_map&& srcs) = 0;

        virtual void destroy_task(io::yield_ctx&, const uuid& u) = 0;

        // if owner is supplied, the tree may
        // be bound to that context for calls
        // if it would not otherwise have an owning context
        virtual std::shared_ptr<node>
            fetch(io::yield_ctx&, const uuid& uuid, 
                    context_ptr owner=context_ptr()) const = 0;

        // returns null if subscription failed
        virtual subscription_ptr
            subscribe(io::yield_ctx&,
                    const uuid& ctx, const std::vector<std::string_view>& path,
                    float min_interval, float max_interval, float timeout) = 0;

        // returns invalid value if call failed
        virtual value call(io::yield_ctx&, const uuid& ctx, 
                const std::vector<std::string_view>& path, value val, float timeout) = 0;

        virtual std::unique_ptr<data_query> query_data(io::yield_ctx&,
                    const uuid& ctx, const std::vector<std::string_view>& path) const = 0;

        virtual bool write_data(io::yield_ctx&,
                    const uuid& ctx, const std::vector<std::string_view>& path,
                    const std::vector<data_point>& data) = 0;

        virtual void mount(io::yield_ctx&, const uuid& src, const uuid& tgt) = 0;
        virtual void unmount(io::yield_ctx&, const uuid& src,  const uuid& tgt) = 0;

        // task-related operations
        virtual void start_task(io::yield_ctx&, const uuid& task) = 0;
        virtual void stop_task(io::yield_ctx&, const uuid& task) = 0;
        virtual params_stream_ptr query_task(io::yield_ctx&, const uuid& task, const params& p) = 0;
    };

    class task : public std::enable_shared_from_this<task> {
    public:
        inline task(io::io_context& ioc, 
                uuid id, const std::string_view& name, 
                const std::string_view& type, const params& p) : 
                    ioc_(ioc), uuid_(id), 
                    name_(name), type_(type), params_(p) {}

        constexpr io::io_context& get_executor() { return ioc_; }

        virtual std::shared_ptr<namespace_> get_namespace() = 0;
        virtual std::shared_ptr<const namespace_> get_namespace() const = 0;

        const std::string& get_name() const { return name_; }
        const std::string& get_type() const { return type_; }
        const params& get_params() const { return params_; }
        const uuid& get_uuid() const { return uuid_; }

        virtual void start(io::yield_ctx&) = 0;
        virtual void stop(io::yield_ctx&) = 0;

        virtual params_stream_ptr query(io::yield_ctx&, const params& p) = 0;

        virtual void destroy(io::yield_ctx& y) = 0;

        signal<> destroyed;
    protected:
        io::io_context& ioc_;
        uuid uuid_; // might be set after object creation...
        const std::string name_;
        const std::string type_;
        const params params_;
    };

    class context : public std::enable_shared_from_this<context> {
    public:
        inline context(io::io_context& ioc, 
                const uuid& uuid, const std::string_view& name, 
                const std::string_view& type, const params& p) : 
                    ioc_(ioc), uuid_(uuid), name_(name),
                    type_(type), params_(p) {}

        // mount-querying functions
        virtual collection_ptr<mount_info> mounts(bool srcs=true, bool tgts=true) const = 0;

        constexpr io::io_context& get_executor() { return ioc_; }

        virtual std::shared_ptr<namespace_> get_namespace() = 0;
        virtual std::shared_ptr<const namespace_> get_namespace() const = 0;

        const std::string& get_name() const { return name_; }
        const std::string& get_type() const { return type_; }
        const params& get_params() const { return params_; }
        const uuid& get_uuid() const { return uuid_; }

        virtual std::shared_ptr<node> fetch(io::yield_ctx& ctx) = 0;

        // tree manipulation functions
        virtual subscription_ptr  subscribe(io::yield_ctx& ctx, 
                                const std::vector<std::string_view>& variable,
                                float min_interval, float max_interval,
                                float timeout) = 0;

        virtual value call(io::yield_ctx& ctx, action* a, value v, float timeout) = 0;
        virtual value call(io::yield_ctx& ctx, const std::vector<std::string_view>& a, 
                            value v, float timeout) = 0;

        virtual bool write_data(io::yield_ctx& yield, variable* v, 
                                    const std::vector<data_point>& data) = 0;
        virtual bool write_data(io::yield_ctx& yield, const std::vector<std::string_view>& var,
                                    const std::vector<data_point>& data) = 0;

        virtual data_query_ptr query_data(io::yield_ctx& yield, 
                                                          const node* n) const = 0;
        virtual data_query_ptr query_data(io::yield_ctx& yield, 
                                          const std::vector<std::string_view>& n) const = 0;

        virtual void mount(io::yield_ctx& ctx, const std::shared_ptr<context>& src) = 0;
        virtual void unmount(io::yield_ctx& ctx, const std::shared_ptr<context>& src) = 0;

        virtual void destroy(io::yield_ctx& yield) = 0;
        signal<> destroyed;
    protected:
        io::io_context& ioc_;
        const uuid uuid_;
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
    template<>
        struct collection_key<std::shared_ptr<task>> {
            typedef uuid type;
            static uuid get(const std::shared_ptr<task>& t) {
                return t->get_uuid(); 
            }
        };

    // add equality operator the mount_info type
    inline bool operator==(const mount_info& lh, const mount_info& rh) {
        return lh.src == rh.src && lh.tgt == rh.tgt;
    }
    inline bool operator!=(const mount_info& lh, const mount_info& rh) {
        return lh.src != rh.src || lh.tgt != rh.tgt;
    }
}

namespace std {
    // make the mount type hashable
    // so we can put it in a query
    template<>
        struct hash<telegraph::mount_info> {
            std::size_t operator()(const telegraph::mount_info&i) const {
                return hash<telegraph::uuid>()(i.src) ^ 
                       (hash<telegraph::uuid>()(i.tgt) << 1);
            }
        };
}

#endif
