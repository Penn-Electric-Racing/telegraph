#ifndef __TELEGRAPH_NAMESPACE_HPP__
#define __TELEGRAPH_NAMESPACE_HPP__

#include "query.hpp"
#include "value.hpp"
#include "data.hpp"

#include "../utils/info.hpp"
#include "../utils/uuid.hpp"
#include "../utils/io_fwd.hpp"

#include <memory>

namespace telegraph {
    class namespace_;
    class variable;
    class action;
    class node;

    class subscription;
    using subscription_ptr = std::shared_ptr<subscription>;

    class data_point;
    class data_query;

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

    // underscore is to not conflict with builtin
    // namespace token
    class namespace_ {
    public:
        constexpr namespace_(const uuid& uuid) : uuid_(uuid) {}

        const uuid& get_uuid() const { return uuid_; }

        virtual query_ptr<mount_info> mounts(io::yield_ctx&,
                    const uuid& srcs_of=uuid(),
                    const uuid& tgts_of=uuid()) const = 0;

        virtual query_ptr<context_ptr> contexts(io::yield_ctx&, 
                    const uuid& by_uuid=uuid(), 
                    const std::string& by_name=std::string(), 
                    const std::string& by_type=std::string()) const = 0;

        virtual query_ptr<task_ptr> tasks(io::yield_ctx&, const uuid& by_uuid=uuid(),
                    const std::string& by_name=std::string(), 
                    const std::string& by_type=std::string()) const = 0;

        // if owner is supplied, the tree may
        // be bound to that context for calls
        // if it would not otherwise have an owning context
        virtual std::shared_ptr<node>
            fetch(io::yield_ctx&, const uuid& uuid, 
                    context_ptr owner=context_ptr()) const = 0;

        virtual subscription_ptr
            subscribe(io::yield_ctx&,
                    const uuid& ctx, const std::vector<std::string>& path,
                    int32_t min_interval, int32_t max_interval) = 0;

        virtual value call(io::yield_ctx&, 
                    const uuid& ctx, const std::vector<std::string>& path,
                    const value& val) = 0;

        virtual std::unique_ptr<data_query> data(io::yield_ctx&,
                    const uuid& ctx, const std::vector<std::string>& path) const = 0;

        virtual bool write_data(io::yield_ctx&,
                    const uuid& ctx, const std::vector<std::string>& path,
                    const std::vector<data_point>& data) = 0;

        virtual bool mount(io::yield_ctx&, const uuid& src, const uuid& tgt) = 0;
        virtual bool unmount(io::yield_ctx&, const uuid& src,  const uuid& tgt) = 0;
    protected:
        uuid uuid_;
    };

    class task : public std::enable_shared_from_this<task> {
    public:
        inline task(uuid id, const std::string& name, 
                const std::string& type, const info& i) : 
            uuid_(id), name_(name), type_(type), info_(i) {}

        const std::string& get_name() const { return name_; }
        const std::string& get_type() const { return type_; }
        const info& get_info() const { return info_; }
        const uuid& get_uuid() const { return uuid_; }

        // queue the task into the io_context
        virtual void start(io::yield_ctx&, io::io_context& ioc, const info& info) = 0;
        virtual void stop(io::yield_ctx&, const info& info) = 0;

        virtual void destroy(io::yield_ctx&) = 0;
    private:
        const uuid uuid_;
        const std::string name_;
        const std::string type_;
        const info info_;
    };

    class context : public std::enable_shared_from_this<context> {
    public:
        inline context(const uuid& uuid, const std::string& name, 
                const std::string& type, const info& i) : 
                    uuid_(uuid), name_(name),
                    type_(type), info_(i) {}

        virtual namespace_* get_namespace() = 0;
        virtual const namespace_* get_namespace() const = 0;

        const std::string& get_name() const { return name_; }
        const std::string& get_type() const { return type_; }
        const info& get_info() const { return info_; }
        const uuid& get_uuid() const { return uuid_; }


        virtual std::shared_ptr<node> fetch(io::yield_ctx& ctx) = 0;

        // tree manipulation functions
        inline virtual subscription_ptr  subscribe(io::yield_ctx& ctx, variable* v, 
                                int32_t min_interval, int32_t max_interval) = 0;
        inline virtual subscription_ptr  subscribe(io::yield_ctx& ctx, 
                                const std::vector<std::string>& variable,
                                int32_t min_interval, int32_t max_interval) = 0;

        inline virtual value call(io::yield_ctx& ctx, 
                                    action* a, const value& v) = 0;
        inline virtual value call(io::yield_ctx& ctx, 
                        const std::vector<std::string>& a, const value& v) = 0;

        inline virtual bool write_data(io::yield_ctx& yield, variable* v, 
                                    const std::vector<data_point>& data) = 0;
        inline virtual bool write_data(io::yield_ctx& yield, const std::vector<std::string>& var,
                                    const std::vector<data_point>& data) = 0;

        inline virtual std::unique_ptr<data_query> query_data(io::yield_ctx& yield, 
                                                          const node* n) const = 0;
        inline virtual std::unique_ptr<data_query> query_data(io::yield_ctx& yield, 
                                          const std::vector<std::string>& n) const = 0;

        // mount-querying functions
        inline virtual query_ptr<mount_info> mounts(io::yield_ctx& yield, 
                                        bool srcs=true, bool tgts=true) const = 0;

        inline virtual bool mount(io::yield_ctx& ctx, 
                            const std::shared_ptr<context>& src) = 0;
        inline virtual bool unmount(io::yield_ctx& ctx, 
                            const std::shared_ptr<context>& src) = 0;

        inline virtual bool destroy(io::yield_ctx& yield) = 0;
    private:
        const uuid uuid_;
        const std::string name_;
        const std::string type_;
        const info info_;
    };

    // query_key specialization
    // for context/info so that we can do get() on a query
    // to get by the uuid
    template<>
        struct query_key<std::shared_ptr<context>> {
            typedef uuid type;
            static uuid get(const std::shared_ptr<context>& c) { 
                return c->get_uuid(); 
            }
        };
    template<>
        struct query_key<std::shared_ptr<task>> {
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
