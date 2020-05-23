#include "tmp_archive.hpp"

#include "../common/nodes.hpp"
#include "../common/data.hpp"

#include <string_view>

#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace telegraph {

    tmp_archive::tmp_archive(io::io_context& ioc, const std::string_view& name,
                            std::unique_ptr<node>&& src)
            : local_context(ioc, name, "tmp_archive", params{}, std::move(src)) {}

    tmp_archive::~tmp_archive() {
        for (auto i : recordings_) i.second->cancelled.remove(this);
        for (auto i : recordings_queries_) {
            auto r = i.second.lock();
            if (r) {
                r->destroyed.remove(this);
                r->close();
            }
        }
    }

    void
    tmp_archive::record(variable* v, subscription_ptr s) {
        if (!v) return;
        recordings_[v] = s;

        params obj = params::object();
        obj["event"] = "record";
        obj["path"] = params{v->path()};
        for (auto rq : recordings_queries_) {
            auto sp = rq.second.lock();
            if (sp) sp->write(params{obj});
        }

    }

    void
    tmp_archive::record_stop(variable* v) {
        if (!v) return;
        recordings_.erase(v);

        params obj = params::object();
        obj["event"] = "record_stop";
        obj["path"] = params{v->path()};
        for (auto rq : recordings_queries_) {
            auto sp = rq.second.lock();
            if (sp) sp->write(params{obj});
        }
    }

    params_stream_ptr
    tmp_archive::request(io::yield_ctx& yield,
                        const params& p) {
        if (p.is_object()) {
            const std::string& s = p.at("type").get<std::string>();
            if (s == "record") {
                const std::string& uuids = p.at("uuid").get<std::string>();
                uuid ctx_u = boost::lexical_cast<uuid>(uuids);
                auto ns = ns_.lock();
                if (!ns) return nullptr;
                auto ctx = ns->contexts->get(ctx_u);
                if (!ctx) return nullptr;

                std::vector<std::string_view> path;
                const std::vector<params>& ppath = p.at("var").get<std::vector<params>>();
                for (const params& p : ppath) {
                    path.push_back(p.get<std::string>());
                }
                auto v = dynamic_cast<variable*>(tree_->from_path(path));
                if (!v) return nullptr;
                float min_interval = p.at("min_interval").get<float>();
                float max_interval = p.at("max_interval").get<float>();
                // do the subscribe
                auto s = ctx->subscribe(yield, path, min_interval, max_interval, 1);
                if (!s) return nullptr;
                record(v, s);

                params_stream_ptr p = std::make_shared<params_stream>();
                p->write(params{true});
                p->close();
                return p;
            } else if (s == "record_stop") {
                std::vector<std::string_view> path;
                const std::vector<params>& ppath = p.at("var").get<std::vector<params>>();
                for (const params& p : ppath) {
                    path.push_back(p.get<std::string>());
                }
                auto v = dynamic_cast<variable*>(tree_->from_path(path));
                if (!v) return nullptr;
                record_stop(v);

                params_stream_ptr p = std::make_shared<params_stream>();
                p->write(params{true});
                p->close();
                return p;
            } else if (s == "recordings") {
                params_stream_ptr p = std::make_shared<params_stream>();
                params_stream* raw = p.get();

                std::shared_ptr<tmp_archive> sp = std::static_pointer_cast<tmp_archive>(shared_from_this());
                std::weak_ptr<tmp_archive> wp{sp};
                p->destroyed.add(this, [wp, raw]() {
                    auto sp = wp.lock();
                    if (sp) sp->recordings_queries_.erase(raw);
                });
                recordings_queries_.emplace(p.get(), p);
                // send back currently recording
                for (auto i : recordings_) {
                    std::vector<std::string> path = i.first->path();
                    params obj = params::object();
                    obj["event"] = "recording";
                    obj["path"] = params{path};
                    p->write(std::move(obj));
                }
                return p;
            }
        }
        return nullptr;
    }

    local_context_ptr
    tmp_archive::create(io::yield_ctx& yield, io::io_context& ioc,
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
        return std::make_shared<tmp_archive>(ioc, name, std::move(n));
    }
}