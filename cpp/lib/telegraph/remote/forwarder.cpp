#include "forwarder.hpp"

#include "connection.hpp"
#include "../common/namespace.hpp"
#include "../common/nodes.hpp"

#include "api.pb.h"

#include "api.pb.h"
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <string_view>

namespace telegraph {

    forwarder::forwarder(connection& conn, const std::shared_ptr<namespace_>& ns)
        : conn_(conn), ns_(ns) {
        if (!ns_) return;
        // set the handlers
        conn_.set_handler(api::Packet::kQueryNs, 
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_query_ns(c, p); });
        conn_.set_handler(api::Packet::kStreamComponent,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_stream_component(c, p); });
        conn_.set_handler(api::Packet::kStreamContext,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_stream_context(c, p); });

        conn_.set_handler(api::Packet::kFetchTree, 
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_fetch_tree(c, p); });

        conn_.set_handler(api::Packet::kMount,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_mount(c, p); });
        conn_.set_handler(api::Packet::kUnmount,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_unmount(c, p); });

        conn_.set_handler(api::Packet::kCreateContext,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_create_context(c, p); });
        conn_.set_handler(api::Packet::kCreateComponent,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_create_component(c, p); });
        conn_.set_handler(api::Packet::kDestroyContext,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_destroy_context(c, p); });
        conn_.set_handler(api::Packet::kDestroyComponent,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_destroy_component(c, p); });

        conn_.set_handler(api::Packet::kSubChange,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_sub_change(c, p); });
        conn_.set_handler(api::Packet::kCallAction,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_call_action(c, p); });

        conn_.set_handler(api::Packet::kDataWrite,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_data_write(c, p); });
        conn_.set_handler(api::Packet::kDataQuery,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_data_query(c, p); });

    }

    forwarder::~forwarder() {
        // unset the handlers for context added/removed
        if (!ns_) return;

        auto c = ns_->contexts;
        auto t = ns_->components;
        auto m = ns_->mounts;

        c->added.remove(this);
        c->removed.remove(this);
        t->added.remove(this);
        t->removed.remove(this);
        m->added.remove(this);
        m->removed.remove(this);

        // we need to also avoid that during destruction
        // a message is sent out. that will try and elongate
        // the lifetime of the underlying connection, which can't
        // be done at this point (because the connection is closed)
        for (auto& s : subs_) {
            s.second->cancelled.remove(this);
            s.second->data.remove(this);
        }
        for (auto& s : streams_) {
            s.second->reset_pipe(); // stop forwarding info
        }
        for (auto& s : queries_) {
            s.second->data.remove(this);
        }
    }

    void
    forwarder::reply_error(const api::Packet& p, const std::exception& e) {
        api::Packet res;
        res.set_error(e.what());
        conn_.write_back(p.req_id(), std::move(res));
    }

    void
    forwarder::handle_query_ns(io::yield_ctx& yield, const api::Packet& p) {
        int32_t req_id = p.req_id();
        api::Packet res;
        api::Namespace* ns = res.mutable_ns();

        auto c = ns_->contexts;
        auto t = ns_->components;
        auto m = ns_->mounts;

        // dump all the contexts/components/mounts
        for (const auto& i : *c) {
            auto& ctx = i.second;
            std::string uuid = boost::lexical_cast<std::string>(ctx->get_uuid());

            api::Context* ct = ns->add_contexts();
            ct->set_uuid(std::move(uuid));
            ct->set_name(ctx->get_name());
            ct->set_type(ctx->get_type());
            api::Params* p = ct->mutable_params();
            ctx->get_params().pack(p);
        }
        for (const auto& i : *t) {
            auto& component = i.second;
            std::string uuid = boost::lexical_cast<std::string>(component->get_uuid());

            api::Component* ta = ns->add_components();
            ta->set_uuid(std::move(uuid));
            ta->set_name(component->get_name());
            ta->set_type(component->get_type());
            api::Params* p = ta->mutable_params();
            component->get_params().pack(p);
        }
        for (const auto& i : *m) {
            auto& mount = i.second;
            auto s = mount.src.lock();
            auto t = mount.tgt.lock();
            std::string src = boost::lexical_cast<std::string>(s->get_uuid());
            std::string tgt = boost::lexical_cast<std::string>(t->get_uuid());

            api::Mount* mt = ns->add_mounts();
            mt->set_src(std::move(src));
            mt->set_tgt(std::move(tgt));
        }

        conn_.write_back(p.req_id(), std::move(res));

        c->added.add(this, [this, req_id] (const context_ptr& ctx) {
            api::Packet res;
            api::Context* c = res.mutable_context_added();
            std::string uuid = boost::lexical_cast<std::string>(ctx->get_uuid());
            c->set_uuid(std::move(uuid));
            c->set_name(ctx->get_name());
            c->set_type(ctx->get_type());
            api::Params* p = c->mutable_params();
            ctx->get_params().pack(p);

            conn_.write_back(req_id, std::move(res));
        });
        c->removed.add(this, [this, req_id] (const context_ptr& ctx) {
            api::Packet res;
            std::string u = boost::lexical_cast<std::string>(ctx->get_uuid());
            res.set_context_removed(std::move(u));
            conn_.write_back(req_id, std::move(res));
        });

        t->added.add(this, [this, req_id] (const component_ptr& component) {
            api::Packet res;
            api::Component* t = res.mutable_component_added();
            std::string uuid = boost::lexical_cast<std::string>(component->get_uuid());
            t->set_uuid(std::move(uuid));
            t->set_name(component->get_name());
            t->set_type(component->get_type());
            api::Params* p = t->mutable_params();
            component->get_params().pack(p);
            conn_.write_back(req_id, std::move(res));
        });
        t->removed.add(this, [this, req_id] (const component_ptr& component) {
            api::Packet res;
            std::string u = boost::lexical_cast<std::string>(component->get_uuid());
            res.set_component_removed(std::move(u));
            conn_.write_back(req_id, std::move(res));
        });
        m->added.add(this, [this, req_id] (const mount_info& m) {
            api::Packet res;
            auto s = m.src.lock();
            auto t = m.tgt.lock();
            if (!s || !t) return;
            std::string src = boost::lexical_cast<std::string>(s->get_uuid());
            std::string tgt = boost::lexical_cast<std::string>(t->get_uuid());

            api::Mount* mt = res.mutable_mount_added();
            mt->set_src(std::move(src));
            mt->set_tgt(std::move(tgt));
            conn_.write_back(req_id, std::move(res));
        });
        m->removed.add(this, [this, req_id] (const mount_info& m) {
            api::Packet res;
            auto s = m.src.lock();
            auto t = m.tgt.lock();
            if (!s || !t) return;
            std::string src = boost::lexical_cast<std::string>(s->get_uuid());
            std::string tgt = boost::lexical_cast<std::string>(t->get_uuid());

            api::Mount* mt = res.mutable_mount_removed();
            mt->set_src(std::move(src));
            mt->set_tgt(std::move(tgt));
            conn_.write_back(req_id, std::move(res));
        });
    }

    void
    forwarder::handle_fetch_tree(io::yield_ctx& yield, const api::Packet& p) {
        try {
            uuid ctx_uuid = boost::lexical_cast<uuid>(p.fetch_tree());

            auto ctx = ns_->contexts->get(ctx_uuid);
            if (!ctx) throw remote_error("no such context");
            std::shared_ptr<node> n = ctx->fetch(yield);
            api::Packet res;
            if (!n) {
                res.set_success(false);
            } else {
                Node* proto = res.mutable_fetched_tree();
                n->pack(proto);
            }
            conn_.write_back(p.req_id(), std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_sub_change(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& cs = p.sub_change();

            uuid ctx_uuid = boost::lexical_cast<uuid>(cs.uuid());
            float db = cs.debounce();
            float rf = cs.refresh();
            float timeout = cs.timeout();

            auto it = subs_.find(req_id);

            if (it == subs_.end()) {
                // new subscription!
                std::vector<std::string_view> path;
                for (const auto& s : cs.variable()) {
                    path.push_back(s);
                }
                auto ctx = ns_->contexts->get(ctx_uuid);
                if (!ctx) throw missing_error("no such context");
                auto sub = ctx->subscribe(c, path, db, rf, timeout);
                if (!sub) {
                    api::Packet r;
                    r.set_success(false);
                    conn_.write_back(req_id, std::move(r));
                    return;
                }
                sub->data.add(this, [this, req_id](value v) {
                    // write the data back
                    api::Packet p;
                    v.pack(p.mutable_sub_update());
                    conn_.write_back(req_id, std::move(p));
                });
                sub->cancelled.add(this, [this, req_id]() {
                    subs_.erase(req_id);
                    api::Packet p;
                    p.set_cancel(0);
                    conn_.write_back(req_id, std::move(p));
                    conn_.close_stream(req_id); 
                });
                // handle getting a cancel() message
                conn_.set_stream_cb(req_id, 
                    [this](io::yield_ctx& yield, const api::Packet& p) {
                        if (p.payload_case() == api::Packet::kSubChange) {
                            const api::Subscription& s = p.sub_change();
                            bool success = false;
                            try {
                                subs_.at(p.req_id())->change(yield,
                                        s.debounce(), s.refresh(),
                                        s.timeout());
                                success = true;
                            } catch (...) {}
                            api::Packet r;
                            r.set_success(success);
                            conn_.write_back(p.req_id(), std::move(r));
                        } else if (p.payload_case() == api::Packet::kSubPoll) {
                            subs_.at(p.req_id())->poll();
                        } else if (p.payload_case() == api::Packet::kCancel) {
                            subs_.erase(p.req_id()); // will erase the sub and should trigger cancelled()
                        }
                    });
                // reply with the sub type
                api::Packet reply;
                sub->get_type().pack(reply.mutable_sub_type());
                conn_.write_back(req_id, std::move(reply));

                // put in subs map
                subs_.emplace(std::make_pair(req_id, std::move(sub)));
            } else {
                const auto& sub = it->second;
                sub->change(c, db, rf, timeout);
            }
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_call_action(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.call_action();
            // get the argument/context parameters
            uuid u = boost::lexical_cast<uuid>(req.uuid());
            std::vector<std::string_view> path;
            for (const auto& s : req.action()) {
                path.push_back(s);
            }
            value v{req.value()};
            // make the call
            auto ctx = ns_->contexts->get(u);
            if (!ctx) throw missing_error("no such context");
            value ret = ctx->call(c, path, v, req.timeout());
            // reply with the result
            api::Packet res;
            ret.pack(res.mutable_call_return());
            conn_.write_back(req_id, std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_data_write(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.data_write();

            uuid u = boost::lexical_cast<uuid>(req.uuid());
            std::vector<data_point> data;
            for (const Datapoint& v : req.data()) {
                uint64_t millisecs = v.timestamp();
                std::chrono::milliseconds m{millisecs};
                time_point tp{m};
                data.push_back(data_point{tp,
                               value{v.value()}});
            }
            std::vector<std::string_view> path;
            for (const auto& s : req.path()) {
                path.push_back(s);
            }
            auto ctx = ns_->contexts->get(u);
            if (!ctx) throw missing_error("no such context");
            bool status = ctx->write_data(c, path, data);
            // send response
            api::Packet res;
            res.set_success(status);
            conn_.write_back(req_id, std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_data_query(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.data_query();
            uuid u = boost::lexical_cast<uuid>(req.uuid());
            std::vector<std::string_view> path;
            for (const auto& s : req.path()) {
                path.push_back(s);
            }
            auto ctx = ns_->contexts->get(u);
            if (!ctx) throw missing_error("no such context");
            auto q = ctx->query_data(c, path);
            q->data.add(this, [this, req_id](const std::vector<data_point>& data) {
                api::Packet p;
                api::DataPacket* pack = p.mutable_archive_update();
                for (const data_point& dp : data) {
                    Datapoint* d = pack->add_data();
                    auto dur = dp.get_time().time_since_epoch();
                    uint64_t ts = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
                    d->set_timestamp(ts);
                    dp.get_value().pack(d->mutable_value());
                }
                conn_.write_back(req_id, std::move(p));
            });
            conn_.set_stream_cb(req_id,
                [this](io::yield_ctx& yield, const api::Packet& p) {
                    if (p.payload_case() == api::Packet::kCancel) {
                        queries_.erase(p.req_id());
                    }
                });
            queries_.emplace(req_id, q);
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_stream_context(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.stream_context();
            uuid u = boost::lexical_cast<uuid>(req.uuid());
            params par = params::unpack(req.params(), ns_.get());

            auto ctx = ns_->contexts->get(u);
            if (!ctx) throw missing_error("no such context");
            params_stream_ptr s = ctx->request(c, par);

            if (s) {
                api::Packet res;
                res.set_success(true);
                conn_.write_back(req_id, std::move(res));

                s->set_pipe([this, req_id] (params&& p) {
                    // write an update packet
                    api::Packet update;
                    p.move(update.mutable_stream_update());
                    conn_.write_back(req_id, std::move(update));
                }, [this, req_id]() {
                    conn_.close_stream(req_id);
                    // on close send back a cancel message
                    api::Packet cancel;
                    cancel.set_cancel(0);
                    conn_.write_back(req_id, std::move(cancel));

                    // will delete the stream_ptr (and this object)
                    streams_.erase(req_id);
                });
                streams_.emplace(std::make_pair(req_id, std::move(s)));
                conn_.set_stream_cb(req_id,
                    [this] (io::yield_ctx& yield, const api::Packet& p) {
                        if (p.payload_case() == api::Packet::kCancel) {
                            auto it = streams_.find(p.req_id());
                            if (it == streams_.end()) return;
                            it->second->reset_pipe();
                            it->second->close();
                            streams_.erase(p.req_id());
                        }
                    });
            } else {
                api::Packet res;
                res.set_success(false);
                conn_.write_back(req_id, std::move(res));
            }
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_stream_component(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.stream_component();
            uuid u = boost::lexical_cast<uuid>(req.uuid());
            params par = params::unpack(req.params(), ns_.get());

            auto component = ns_->components->get(u);
            if (!component) throw missing_error("no such component");
            params_stream_ptr s = component->request(c, par);

            if (s) {
                api::Packet res;
                res.set_success(true);
                conn_.write_back(req_id, std::move(res));

                conn_.set_stream_cb(req_id,
                    [this] (io::yield_ctx& yield, const api::Packet& p) {
                        if (p.payload_case() == api::Packet::kCancel) {
                            auto it = streams_.find(p.req_id());
                            if (it == streams_.end()) return;
                            it->second->close();
                        }
                    });
                s->set_pipe([this, req_id] (params&& p) {
                    // write an update packet
                    api::Packet update;
                    p.move(update.mutable_stream_update());
                    conn_.write_back(req_id, std::move(update));
                }, [this, req_id]() {
                    conn_.close_stream(req_id);
                    // on close send back a cancel message
                    api::Packet cancel;
                    cancel.set_cancel(0);
                    conn_.write_back(req_id, std::move(cancel));

                    // will delete the stream_ptr (and this object)
                    streams_.erase(req_id);
                });
                if (!s->is_closed()) {
                    streams_.emplace(std::make_pair(req_id, std::move(s)));
                }
            } else {
                api::Packet res;
                res.set_success(false);
                conn_.write_back(req_id, std::move(res));
            }
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_mount(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.mount();
            uuid src = boost::lexical_cast<uuid>(req.src());
            uuid tgt = boost::lexical_cast<uuid>(req.tgt());

            // get the contexts by uuid
            auto s = ns_->contexts->get(src);
            auto t = ns_->contexts->get(tgt);
            if (!s || !t) throw missing_error("no such contexts");
            t->mount(c, s);

            api::Packet res;
            res.set_success(true);
            conn_.write_back(req_id, std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_unmount(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.unmount();
            uuid src = boost::lexical_cast<uuid>(req.src());
            uuid tgt = boost::lexical_cast<uuid>(req.tgt());

            auto s = ns_->contexts->get(src);
            auto t = ns_->contexts->get(tgt);
            if (!s || !t) throw missing_error("no such contexts");
            t->unmount(c, s);

            api::Packet res;
            res.set_success(true);
            conn_.write_back(req_id, std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_create_context(io::yield_ctx& yield, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& c = p.create_context();
            const std::string& name = c.name();
            const std::string& type = c.type();
            params par = params::unpack(c.params(), ns_.get());
            context_ptr n = ns_->create_context(yield, name, type, par);
            api::Packet res;
            if (!n) {
                res.set_success(false);
            } else {
                std::string u = boost::lexical_cast<std::string>(n->get_uuid());
                res.set_context_created(std::move(u));
            }
            conn_.write_back(req_id, std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_create_component(io::yield_ctx& yield, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& c = p.create_component();
            const std::string& name = c.name();
            const std::string& type = c.type();
            params par = params::unpack(c.params(), ns_.get());
            component_ptr n = ns_->create_component(yield, name, type, par);
            api::Packet res;
            if (!n) {
                res.set_success(false);
            } else {
                std::string u = boost::lexical_cast<std::string>(n->get_uuid());
                res.set_component_created(std::move(u));
            }
            conn_.write_back(req_id, std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_destroy_context(io::yield_ctx& yield, const api::Packet& p) {
        try {
            uuid u = boost::lexical_cast<uuid>(p.destroy_context());
            ns_->destroy_context(yield, u);

            api::Packet res;
            res.set_success(true);
            conn_.write_back(p.req_id(), std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_destroy_component(io::yield_ctx& yield, const api::Packet& p) {
        try {
            uuid u = boost::lexical_cast<uuid>(p.destroy_component());
            ns_->destroy_component(yield, u);

            api::Packet res;
            res.set_success(true);
            conn_.write_back(p.req_id(), std::move(res));
        } catch (const std::exception& e) {
            reply_error(p, e);
        }
    }
}
