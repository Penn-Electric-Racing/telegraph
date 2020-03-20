#include "forwarder.hpp"

#include "connection.hpp"
#include "../common/namespace.hpp"
#include "../common/nodes.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace telegraph {

    forwarder::forwarder(connection& conn, const std::shared_ptr<namespace_>& ns)
        : conn_(conn), ns_(ns) {
        if (!ns_) return;
        // set the handlers
        conn_.set_handler(api::Packet::kQueryNs, 
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_query_ns(c, p); });
        conn_.set_handler(api::Packet::kQueryTask,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_query_task(c, p); });
        conn_.set_handler(api::Packet::kQueryContext,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_query_context(c, p); });

        conn_.set_handler(api::Packet::kFetchTree, 
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_fetch_tree(c, p); });

        conn_.set_handler(api::Packet::kMount,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_mount(c, p); });
        conn_.set_handler(api::Packet::kUnmount,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_unmount(c, p); });

        conn_.set_handler(api::Packet::kCreateContext,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_create_context(c, p); });
        conn_.set_handler(api::Packet::kCreateTask,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_create_task(c, p); });
        conn_.set_handler(api::Packet::kDestroyContext,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_destroy_context(c, p); });
        conn_.set_handler(api::Packet::kDestroyTask,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_destroy_task(c, p); });

        conn_.set_handler(api::Packet::kChangeSub,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_change_sub(c, p); });
        conn_.set_handler(api::Packet::kCallAction,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_call_action(c, p); });

        conn_.set_handler(api::Packet::kDataWrite,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_data_write(c, p); });
        conn_.set_handler(api::Packet::kDataQuery,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_data_query(c, p); });

        conn_.set_handler(api::Packet::kStartTask,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_start_task(c, p); });
        conn_.set_handler(api::Packet::kStopTask,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_stop_task(c, p); });

    }

    forwarder::~forwarder() {
        // unset the handlers for context added/removed
        if (!ns_) return;

        auto c = ns_->contexts;
        auto t = ns_->tasks;
        auto m = ns_->mounts;

        c->added.remove(this);
        c->removed.remove(this);
        t->added.remove(this);
        t->removed.remove(this);
        m->added.remove(this);
        m->removed.remove(this);
    }

    void
    forwarder::reply_error(const api::Packet& p, const error& e) {
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
        auto t = ns_->tasks;
        auto m = ns_->mounts;

        // dump all the contexts/tasks/mounts
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
            auto& task = i.second;
            std::string uuid = boost::lexical_cast<std::string>(task->get_uuid());

            api::Task* ta = ns->add_tasks();
            ta->set_uuid(std::move(uuid));
            ta->set_name(task->get_name());
            ta->set_type(task->get_type());
            api::Params* p = ta->mutable_params();
            task->get_params().pack(p);
        }
        for (const auto& i : *m) {
            auto& mount = i.second;
            std::string src = boost::lexical_cast<std::string>(mount.src);
            std::string tgt = boost::lexical_cast<std::string>(mount.tgt);

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

        t->added.add(this, [this, req_id] (const task_ptr& task) {
            api::Packet res;
            api::Task* t = res.mutable_task_added();
            std::string uuid = boost::lexical_cast<std::string>(task->get_uuid());
            t->set_uuid(std::move(uuid));
            t->set_name(task->get_name());
            t->set_type(task->get_type());
            api::Params* p = t->mutable_params();
            task->get_params().pack(p);
            conn_.write_back(req_id, std::move(res));
        });
        t->removed.add(this, [this, req_id] (const task_ptr& task) {
            api::Packet res;
            std::string u = boost::lexical_cast<std::string>(task->get_uuid());
            res.set_task_removed(std::move(u));
            conn_.write_back(req_id, std::move(res));
        });
        m->added.add(this, [this, req_id] (const mount_info& m) {
            api::Packet res;
            std::string src = boost::lexical_cast<std::string>(m.src);
            std::string tgt = boost::lexical_cast<std::string>(m.tgt);

            api::Mount* mt = res.mutable_mount_added();
            mt->set_src(std::move(src));
            mt->set_tgt(std::move(tgt));
            conn_.write_back(req_id, std::move(res));
        });
        m->removed.add(this, [this, req_id] (const mount_info& m) {
            api::Packet res;
            std::string src = boost::lexical_cast<std::string>(m.src);
            std::string tgt = boost::lexical_cast<std::string>(m.tgt);

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

            std::shared_ptr<node> n = ns_->fetch(yield, ctx_uuid);
            if (!n) throw remote_error("unable to fetch context");
            api::Packet res;
            Node* proto = res.mutable_fetched_tree();
            n->pack(proto);
            conn_.write_back(p.req_id(), std::move(res));
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_change_sub(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& cs = p.change_sub();

            uuid ctx_uuid = boost::lexical_cast<uuid>(cs.uuid());
            float min_int = cs.min_interval();
            float max_int = cs.max_interval();
            float timeout = cs.max_interval();

            auto it = subs_.find(req_id);

            if (it == subs_.end()) {
                // new subscription!
                std::vector<std::string_view> path;
                for (const auto& s : cs.variable()) {
                    path.push_back(s);
                }
                auto sub = ns_->subscribe(c, ctx_uuid, path, min_int, max_int, timeout);
                sub->data.add([this, req_id](value v) {
                    // write the data back
                    api::Packet p;
                    v.pack(p.mutable_variable_update());
                    conn_.write_back(req_id, std::move(p));
                });
                sub->cancelled.add([this, req_id]() {
                    api::Packet p;
                    p.set_cancel(0);
                    conn_.write_back(req_id, std::move(p));
                    conn_.close_stream(req_id); 
                });
                // handle getting a cancel() message
                conn_.set_stream_cb(req_id, 
                    [this](io::yield_ctx& yield, const api::Packet& p) {
                        if (p.payload_case() == api::Packet::kCancel) {
                            subs_.erase(p.req_id()); // will erase the sub and should trigger cancelled()
                        }
                    });
                // reply with the sub type
                api::Packet reply;
                sub->get_type().pack(reply.mutable_sub_type());

                // put in subs map
                subs_.emplace(std::make_pair(req_id, std::move(sub)));
            } else {
                const auto& sub = it->second;
                sub->change(c, min_int, max_int, timeout);
            }
        } catch (const error& e) {
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
            value ret = ns_->call(c, u, path, v, req.timeout());
            // reply with the result
            api::Packet res;
            ret.pack(res.mutable_call_return());
            conn_.write_back(req_id, std::move(res));
        } catch (const error& e) {
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
                data.push_back(data_point{v.timestamp(), value{v.value()}});
            }
            std::vector<std::string_view> path;
            for (const auto& s : req.path()) {
                path.push_back(s);
            }
            bool status = ns_->write_data(c, u, path, data);
            // send response
            api::Packet res;
            res.set_success(status);
            conn_.write_back(req_id, std::move(res));
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_data_query(io::yield_ctx& c, const api::Packet& p) {
        try {
            throw io_error("not yet implemented on server side!");
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_start_task(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            uuid u = boost::lexical_cast<uuid>(p.start_task());
            ns_->start_task(c, u);
            api::Packet res;
            res.set_success(true);
            conn_.write_back(req_id, std::move(res));
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_stop_task(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            uuid u = boost::lexical_cast<uuid>(p.stop_task());
            ns_->stop_task(c, u);
            api::Packet res;
            res.set_success(true);
            conn_.write_back(req_id, std::move(res));
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_query_context(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.query_context();
            uuid u = boost::lexical_cast<uuid>(req.uuid());
            params p{req.params()};
            params_stream_ptr s = ns_->query_context(c, u, p);

            streams_.emplace(std::make_pair(req_id, std::move(s)));

            s->set_pipe([this, req_id] (params&& p) {
                // write an update packet
                api::Packet update;
                p.move(update.mutable_query_update());
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
            conn_.set_stream_cb(req_id,
                [this] (io::yield_ctx& yield, const api::Packet& p) {
                    if (p.payload_case() == api::Packet::kCancel) {
                        auto it = streams_.find(p.req_id());
                        if (it == streams_.end()) return;
                        it->second->close();
                    }
                });
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_query_task(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& req = p.query_task();
            uuid u = boost::lexical_cast<uuid>(req.uuid());
            params p{req.params()};
            params_stream_ptr s = ns_->query_task(c, u, p);

            streams_.emplace(std::make_pair(req_id, std::move(s)));

            s->set_pipe([this, req_id] (params&& p) {
                // write an update packet
                api::Packet update;
                p.move(update.mutable_query_update());
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
            conn_.set_stream_cb(req_id,
                [this] (io::yield_ctx& yield, const api::Packet& p) {
                    if (p.payload_case() == api::Packet::kCancel) {
                        auto it = streams_.find(p.req_id());
                        if (it == streams_.end()) return;
                        it->second->close();
                    }
                });
        } catch (const error& e) {
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
            ns_->mount(c, src, tgt);
            api::Packet res;
            res.set_success(true);
            conn_.write_back(req_id, std::move(res));
        } catch (const error& e) {
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
            ns_->unmount(c, src, tgt);
            api::Packet res;
            res.set_success(true);
            conn_.write_back(req_id, std::move(res));
        } catch (const error& e) {
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
            params par{c.params()};
            sources_uuid_map m;
            for (const auto& se : c.sources()) {
                if (se.tree_case() == api::SourceEntry::kContext) {
                    // do a query to get the context
                    uuid u = boost::lexical_cast<uuid>(se.context());
                    m.emplace(std::make_pair(se.key(), u));
                } else {
                    std::unique_ptr<node> n{node::unpack(se.root())};
                    m.emplace(std::make_pair(se.key(), std::move(n)));
                }
            }
            context_ptr n = ns_->create_context(yield, name, type, par, std::move(m));
            api::Packet res;
            if (!n) {
                res.set_success(false);
            } else {
                std::string u = boost::lexical_cast<std::string>(n->get_uuid());
                res.set_task_created(std::move(u));
            }
            conn_.write_back(req_id, std::move(res));
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_create_task(io::yield_ctx& yield, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const auto& c = p.create_context();
            const std::string& name = c.name();
            const std::string& type = c.type();
            params par{c.params()};
            sources_uuid_map m;
            for (const auto& se : c.sources()) {
                if (se.tree_case() == api::SourceEntry::kContext) {
                    // do a query to get the context
                    uuid u = boost::lexical_cast<uuid>(se.context());
                    m.emplace(std::make_pair(se.key(), u));
                } else {
                    std::unique_ptr<node> n{node::unpack(se.root())};
                    m.emplace(std::make_pair(se.key(), std::move(n)));
                }
            }
            task_ptr n = ns_->create_task(yield, name, type, par, std::move(m));
            api::Packet res;
            if (!n) {
                res.set_success(false);
            } else {
                std::string u = boost::lexical_cast<std::string>(n->get_uuid());
                res.set_task_created(std::move(u));
            }
            conn_.write_back(req_id, std::move(res));
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_destroy_context(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_destroy_task(io::yield_ctx& c, const api::Packet& p) {
    }
}