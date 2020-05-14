#include "container.hpp"

namespace telegraph {
    container::container(io::io_context& ioc, const std::string_view& name,
                            std::unique_ptr<node>&& tree) : 
                    local_context(ioc, name, "container", params{}, 
                    std::shared_ptr<node>{std::move(tree)}) {}

    params_stream_ptr
    container::request(io::yield_ctx&, const params& p) {
        return nullptr;
    }

    subscription_ptr
    container::subscribe(io::yield_ctx& ctx, 
            const std::vector<std::string_view>& var,
            float min_interval, float max_interval, float timeout) {
        for (const context_ptr& p : mounts_) {
            auto s = p->subscribe(ctx, var, 
                        min_interval, max_interval, timeout);
            if (s) return s;
        }
        return nullptr;
    }

    subscription_ptr
    container::subscribe(io::yield_ctx& ctx,
            const variable* v, float min_interval, float max_interval, float timeout) {
        std::vector<std::string> var = v->path();
        std::vector<std::string_view> varv;
        for (auto& s : var) varv.push_back(s);
        for (const context_ptr& p : mounts_) {
            auto s = p->subscribe(ctx, varv, min_interval, max_interval, timeout);
            if (s) return s;
        }
        return nullptr;
    }

    local_context_ptr
    container::create(io::yield_ctx& yield, io::io_context& ioc,
                const std::string_view& name, const std::string_view& type,
                const params& p, sources_map&& srcs) {
        auto sit = srcs.find("src");
        if (sit == srcs.end()) return nullptr;
        auto& v = sit->second;
        std::unique_ptr<node> n;
        if (v.index() == 0) {
            auto ctx = std::get<context_ptr>(v);
            auto s = ctx->fetch(yield);
            if (!s) return nullptr;
            n = s->clone();
        } else {
            std::unique_ptr<node>& mn = std::get<std::unique_ptr<node>>(v);
            n = std::move(mn);
        }
        return std::make_shared<container>(ioc, name, std::move(n));
    }
}