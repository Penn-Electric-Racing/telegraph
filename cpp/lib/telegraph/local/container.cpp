#include "container.hpp"

namespace telegraph {
    container::container(io::io_context& ioc, const std::string_view& name,
                            std::unique_ptr<node>&& tree) : 
                    local_context(ioc, name, "container", params{}, 
                        std::shared_ptr<node>{std::move(tree)}) {}
    container::~container() {
        for (auto s: subs_) {
            auto sp = s.second.lock();
            if (sp) sp->cancelled.remove(this);
        }
    }

    params_stream_ptr
    container::request(io::yield_ctx& ctx, const params& p) {
        for (const context_ptr& c : mounts_) {
            auto s = c->request(ctx, p);
            if (s) return s;
        }
        return nullptr;
    }

    subscription_ptr
    container::subscribe(io::yield_ctx& ctx, 
            const std::vector<std::string_view>& var,
            float min_interval, float max_interval, float timeout) {
        for (const context_ptr& p : mounts_) {
            auto s = p->subscribe(ctx, var, 
                        min_interval, max_interval, timeout);
            if (s) {
                auto raw = s.get();
                s->cancelled.add(this, [this, raw]() { subs_.erase(raw); });
                subs_.emplace(s.get(), s);
                return s;
            }
        }
        return nullptr;
    }

    subscription_ptr
    container::subscribe(io::yield_ctx& ctx,
            const variable* v, float min_interval, float max_interval, float timeout) {
        std::vector<std::string> var = v->path();
        std::vector<std::string_view> varv;
        for (auto& s : var) varv.push_back(s);
        return subscribe(ctx, varv, min_interval, max_interval, timeout);
    }

    local_context_ptr
    container::create(io::yield_ctx& yield, io::io_context& ioc,
                const std::string_view& name, const std::string_view& type,
                const params& p) {
        auto srcs = p.to_map();
        auto sit = srcs.find("src");
        if (sit == srcs.end()) return nullptr;
        auto& v = sit->second;
        std::unique_ptr<node> n;
        if (v.is_ctx()) {
            auto ctx = v.to_ctx();
            auto s = ctx->fetch(yield);
            if (!s) return nullptr;
            n = s->clone();
        } else if (v.is_tree()) {
            const std::shared_ptr<node>& mn = v.to_tree();
            n = mn->clone();
        }
        if (!n) return nullptr;
        return std::make_shared<container>(ioc, name, std::move(n));
    }
}