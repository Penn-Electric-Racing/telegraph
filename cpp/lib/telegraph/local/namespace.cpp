#include "namespace.hpp"
#include "../common/nodes.hpp"
#include "../utils/errors.hpp"

namespace telegraph {
    local_namespace::local_namespace() 
            : namespace_(rand_uuid()) {
        contexts_ = std::make_shared<query<context_ptr>>();
        tasks_ = std::make_shared<query<task_ptr>>();
        mounts_ = std::make_shared<query<mount_info>>();
    }

    query_ptr<mount_info>
    local_namespace::mounts(io::yield_ctx& yield,
                            const uuid& srcs_of, const uuid& tgts_of) const {
        if (srcs_of.is_nil() && tgts_of.is_nil()) return mounts_;
        return mounts_->chain([srcs_of, tgts_of](const mount_info& m) {
                return srcs_of.is_nil() ? false : (m.tgt == srcs_of) ||
                tgts_of.is_nil() ? false :(m.src == tgts_of);
        });
    }

    query_ptr<context_ptr>
    local_namespace::contexts(io::yield_ctx& yield,
                        const uuid& by_uuid, const std::string& by_name, 
                        const std::string& by_type) const {
        if (by_uuid.is_nil() && by_name.empty() && by_type.empty()) return contexts_;
        return contexts_->chain([by_uuid, by_name, by_type](const context_ptr& p) {
                return (by_uuid.is_nil()  || by_uuid == p->get_uuid()) &&
                       (by_name.empty() || by_name == p->get_name()) &&
                       (by_type.empty() || by_type == p->get_type());
            });
    }

    query_ptr<task_ptr>
    local_namespace::tasks(io::yield_ctx& yield,
                        const uuid& by_uuid, const std::string& by_name,
                        const std::string& by_type) const {
        if (by_uuid.is_nil() && by_name.empty() && by_type.empty()) return tasks_;
        return tasks_->chain([by_uuid, by_name, by_type](const task_ptr& p) {
                return (by_uuid.is_nil() ||  by_uuid == p->get_uuid()) &&
                       (by_name.empty() || by_name == p->get_name()) &&
                       (by_type.empty() || by_type == p->get_type());
            });
    }

    std::shared_ptr<node>
    local_namespace::fetch(io::yield_ctx& yield,
                        const uuid& uuid, context_ptr owner) const {
        return contexts_->has(uuid) ? 
            contexts_->get(uuid)->fetch(yield) : nullptr;
    }

    subscription_ptr
    local_namespace::subscribe(io::yield_ctx& yield,
                    const uuid& ctx, const std::vector<std::string>& path,
                    interval min_interval, interval max_interval, interval timeout) {
        if (!contexts_->has(ctx)) return nullptr;
        auto c = contexts_->get(ctx);
        return c->subscribe(yield, path, min_interval, max_interval, timeout);
    }

    value
    local_namespace::call(io::yield_ctx& yield, const uuid& ctx, 
            const std::vector<std::string>& path, value arg, interval timeout) {
        if (!contexts_->has(ctx)) return value();
        auto c = contexts_->get(ctx);
        return c->call(yield, path, arg, timeout);
    }

    std::unique_ptr<data_query>
    local_namespace::query_data(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path) const {
        if (!contexts_->has(ctx)) return nullptr;
        auto c = contexts_->get(ctx);
        return c->query_data(yield, path);
    }

    bool 
    local_namespace::write_data(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path,
            const std::vector<data_point>& data) {
        auto c = contexts_->get(ctx);
        if (!c) throw missing_error("no such context");
        return c->write_data(yield, path, data);
    }

    void
    local_namespace::mount(io::yield_ctx& yield,
            const uuid& src, const uuid& tgt) {
        auto s = contexts_->get(src);
        auto t = contexts_->get(tgt);
        if (!s || !t) 
            throw missing_error("no such contexts");
        return t->mount(yield, s);
    }

    void
    local_namespace::unmount(io::yield_ctx& yield,
                            const uuid& src, const uuid& tgt) {
        auto s = contexts_->get(src);
        auto t = contexts_->get(tgt);
        if (!s || !t) 
            throw missing_error("no such context");
        return t->unmount(yield, s);
    }


    local_context::local_context(io::io_context& ioc, 
                const std::string& name, const std::string& type,
                const info& i, const std::shared_ptr<node>& tree) : 
                context(ioc, rand_uuid(), name, type, i), tree_(tree), ns_(nullptr) {}

    void
    local_context::reg(io::yield_ctx& yield, local_namespace* ns) {
        if (ns_) throw missing_error("already registered");
        if (!ns) throw missing_error("cannot register null namespace");
        ns_ = ns;
        ns->contexts_->add_(yield, shared_from_this());
    }

    void
    local_context::destroy(io::yield_ctx& yield) {
        if (!ns_) return;
        ns_->contexts_->remove_by_key_(yield, get_uuid());
        ns_ = nullptr;
    }

    query_ptr<mount_info>
    local_context::mounts(io::yield_ctx& yield, bool srcs, bool tgts) const {
        if (!ns_) 
            throw missing_error("not registered");
        return ns_->mounts(yield, srcs || !tgts ? get_uuid() : uuid(), 
                                   tgts ? get_uuid() : uuid());
    }

    void
    local_context::mount(io::yield_ctx& yield, const context_ptr& src) {
        if (!ns_ || !src)
            throw missing_error("not registered");
        mount_info m(src->get_uuid(), get_uuid());
        ns_->mounts_->add_(yield, m);
    }

    void
    local_context::unmount(io::yield_ctx& yield, const context_ptr& src) {
        if (!ns_ || !src) return;
        mount_info m(src->get_uuid(), get_uuid());
        ns_->mounts_->remove_by_key_(yield, m);
        return;
    }

    local_task::local_task(io::io_context& ioc, const std::string& name, 
            const std::string& type, const info& i) 
                : task(ioc, rand_uuid(), name, type, i), ns_(nullptr) {}

    void
    local_task::reg(io::yield_ctx& yield, local_namespace* ns) {
        if (ns_) throw missing_error("task already registered");
        if (!ns) throw missing_error("cannot register task in null namespace");
        ns_ = ns;
        ns_->tasks_->add_(yield, shared_from_this());
    }

    void
    local_task::destroy(io::yield_ctx& yield) {
        if (!ns_) return;
        ns_->tasks_->remove_by_key_(yield, get_uuid());
        ns_ = nullptr;
    }
}
