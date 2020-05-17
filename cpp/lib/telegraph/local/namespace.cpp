#include "namespace.hpp"
#include "../common/nodes.hpp"
#include "../utils/errors.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace telegraph {
    local_namespace::local_namespace(io::io_context& ioc) 
            : namespace_(),
              ioc_(ioc), component_factories_(), context_factories_() {}

    context_ptr 
    local_namespace::create_context(io::yield_ctx& yield, 
                    const std::string_view& name, const std::string_view& type, 
                    const params& p, sources_uuid_map&& srcs) {
        sources_map s;
        for (auto& i : srcs) {
            auto& v = i.second;
            if (v.index() == 0) {
                uuid u = std::get<uuid>(v);
                s.emplace(std::make_pair(i.first, contexts->get(u)));
            } else {
                std::unique_ptr<node>& n = std::get<std::unique_ptr<node>>(v);
                s.emplace(std::make_pair(i.first, std::move(n)));
            }
        }
        auto it = context_factories_.find(type);
        if (it == context_factories_.end()) return nullptr;
        else {
            auto c = (it->second)(yield, ioc_, name, type, p, std::move(s));
            if (c) c->reg(yield, shared_from_this());
            return c;
        }
    }

    void 
    local_namespace::destroy_context(io::yield_ctx& y, const uuid& u) {
        context_ptr c = contexts->get(u);
        if (!c) throw missing_error("No context with uuid " + boost::lexical_cast<std::string>(u));
        c->destroy(y);
    }

    component_ptr 
    local_namespace::create_component(io::yield_ctx& yield, 
                const std::string_view& name, const std::string_view& type, 
                const params& p, sources_uuid_map&& srcs) {
        sources_map s;
        for (auto& i : srcs) {
            auto& v = i.second;
            if (v.index() == 0) {
                uuid u = std::get<uuid>(v);
                s.emplace(std::make_pair(i.first, contexts->get(u)));
            } else {
                std::unique_ptr<node>& n = std::get<std::unique_ptr<node>>(v);
                s.emplace(std::make_pair(i.first, std::move(n)));
            }
        }

        auto it = component_factories_.find(type);
        if (it == component_factories_.end()) return nullptr;
        else {
            auto c = (it->second)(yield, ioc_, name, type, p, std::move(s));
            if (c) c->reg(yield, shared_from_this());
            return c;
        }
    }

    void 
    local_namespace::destroy_component(io::yield_ctx& y, const uuid& u) {
        component_ptr c = components->get(u);
        if (!c) throw missing_error("No component with uuid " + boost::lexical_cast<std::string>(u));
        c->destroy(y);
    }


    local_context::local_context(io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type,
                const params& i, const std::shared_ptr<node>& tree) : 
                context(ioc, rand_uuid(), name, type, i), tree_(tree), ns_() {
        if (tree) {
            tree->set_owner(weak_from_this());
        }
    }

    void
    local_context::reg(io::yield_ctx& yield, const std::shared_ptr<local_namespace>& ns) {
        if (ns_.lock()) throw missing_error("already registered");
        if (!ns) throw missing_error("cannot register null namespace");
        ns_ = ns;
        ns->contexts->add_(shared_from_this());
    }

    void
    local_context::destroy(io::yield_ctx& yield) {
        auto s = ns_.lock();
        if (!s) return;
        // remove all mounts associated with this
        // context
        std::vector<mount_info> mounts;
        for (const auto &m : *s->mounts) {
            if (m.second.tgt.lock().get() == this || 
                m.second.src.lock().get() == this) {
                mounts.push_back(m.second);
            }
        }
        for (const auto& m : mounts) {
            s->mounts->remove_(m);
        }
        s->contexts->remove_by_key_(get_uuid());
        ns_.reset();
        destroyed();
    }

    collection_ptr<mount_info>
    local_context::mounts(bool srcs, bool tgts) const {
        auto s = ns_.lock();
        if (!s) throw missing_error("not registered");
        const uuid& u = uuid_;
        return s->mounts->filter([u, srcs, tgts](const mount_info& i) {
            auto s = i.src.lock();
            auto t = i.tgt.lock();
            if (!s || !t) return false;
            return (srcs && (t->get_uuid() == u)) || (tgts && (s->get_uuid() == u));
        });
    }

    void
    local_context::mount(io::yield_ctx& yield, const context_ptr& src) {
        auto s = ns_.lock();
        if (!s || !src)
            throw missing_error("not registered");
        mount_info m(src, weak_from_this());
        s->mounts->add_(m);
    }

    void
    local_context::unmount(io::yield_ctx& yield, const context_ptr& src) {
        auto s = ns_.lock();
        if (!s || !src) return;
        mount_info m(src, weak_from_this());
        s->mounts->remove_by_key_(m);
    }

    local_component::local_component(io::io_context& ioc, const std::string_view& name, 
            const std::string_view& type, const params& p) 
                : component(ioc, rand_uuid(), name, type, p), ns_() {}

    void
    local_component::reg(io::yield_ctx& yield, const std::shared_ptr<local_namespace>& ns) {
        auto s = ns_.lock();
        if (s) throw missing_error("component already registered");
        if (!ns) throw missing_error("cannot register component in null namespace");
        ns_ = ns;
        ns->components->add_(shared_from_this());
    }

    void
    local_component::destroy(io::yield_ctx& yield) {
        auto s = ns_.lock();
        if (!s) return;
        s->components->remove_by_key_(get_uuid());
        ns_.reset();
        destroyed();
    }
}
