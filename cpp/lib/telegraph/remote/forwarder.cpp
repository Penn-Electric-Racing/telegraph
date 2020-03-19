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
        conn_.set_handler(api::Packet::kFetchTree, 
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_fetch_tree(c, p); });

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
        conn_.set_handler(api::Packet::kQueryTask,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_query_task(c, p); });

        conn_.set_handler(api::Packet::kMount,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_mount(c, p); });
        conn_.set_handler(api::Packet::kUnmount,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_unmount(c, p); });

        conn_.set_handler(api::Packet::kContextsQuery,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_contexts_query(c, p); });
        conn_.set_handler(api::Packet::kMountsQuery,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_mounts_query(c, p); });
        conn_.set_handler(api::Packet::kTasksQuery,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_tasks_query(c, p); });

        conn_.set_handler(api::Packet::kCreateContext,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_create_context(c, p); });
        conn_.set_handler(api::Packet::kCreateTask,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_create_task(c, p); });
        conn_.set_handler(api::Packet::kDestroyContext,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_destroy_context(c, p); });
        conn_.set_handler(api::Packet::kDestroyTask,
                [this] (io::yield_ctx& c, const api::Packet& p) { handle_destroy_task(c, p); });
    }

    forwarder::~forwarder() {
        // unset the handlers
    }

    void
    forwarder::reply_error(const api::Packet& p, const error& e) {
        api::Packet res;
        res.set_error(e.what());
        conn_.write_back(p.req_id(), std::move(res));
    }

    void
    forwarder::handle_query_ns(io::yield_ctx& yield, const api::Packet& p) {
        api::Packet res;
        res.set_ns_uuid(boost::lexical_cast<std::string>(ns_->get_uuid()));
        conn_.write_back(p.req_id(), std::move(res));
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
                p.move(update.mutable_task_update());
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
    forwarder::handle_contexts_query(io::yield_ctx& yield, const api::Packet& p) {
        try {
            const api::ContextsQuery& q = p.contexts_query();
            uuid ctx_uuid = q.uuid().size() > 0 ? boost::lexical_cast<uuid>(q.uuid()) : uuid();
            const std::string& name = q.name();
            const std::string& type = q.type();

            query_ptr<context_ptr> contexts = ns_->contexts(yield, ctx_uuid, name, type);
            if (!contexts) throw remote_error("unable to query contexts");

            // set a bunch of handlers
            api::Packet res;
            api::ContextList* l = res.mutable_context_list();
            for (const auto& i : *contexts) {
                const auto& ctx = i.second;
                api::Context* c = l->add_contexts();
                std::string uuid = boost::lexical_cast<std::string>(ctx->get_uuid());
                c->set_uuid(std::move(uuid));
                c->set_name(ctx->get_name());
                c->set_type(ctx->get_type());
                api::Params* params = c->mutable_params();
                ctx->get_params().pack(params);
            }
            conn_.write_back(p.req_id(), std::move(res));

            // add handlers to the query
            int32_t req_id = p.req_id();
            contexts->added.add(this, [this, req_id] (const context_ptr& ctx) {
                // send back a context added event
                api::Packet p;
                api::Context* c = p.mutable_context_added();
                std::string uuid = boost::lexical_cast<std::string>(ctx->get_uuid());
                c->set_uuid(std::move(uuid));
                c->set_name(ctx->get_name());
                c->set_type(ctx->get_type());
                api::Params* params = c->mutable_params();
                ctx->get_params().pack(params);

                conn_.write_back(req_id, std::move(p));
            });
            contexts->removed.add(this, [this, req_id] (const context_ptr& ctx) {
                std::string uuid = boost::lexical_cast<std::string>(ctx->get_uuid());
                api::Packet p;
                p.set_context_removed(std::move(uuid));
                conn_.write_back(req_id, std::move(p));
            });

            contexts->cancelled.add(this, [this, req_id] () {
                api::Packet cancel;
                cancel.set_cancel(0);
                conn_.write_back(req_id, std::move(cancel));
                conn_.close_stream(req_id);
            });

            // set stream handler to wait for cancel
            // this will own the query, so when
            // the connection is destructed or the stream
            // cancelled, the query will also be destroyed
            conn_.set_stream_cb(req_id,
                [this, contexts] (io::yield_ctx& yield, const api::Packet& p) {
                    if (p.payload_case() == api::Packet::kCancel) {
                        // will trigger cancelled(), which will write back a cancel packet
                        // before closing the stream and removing the captured query ptr
                        contexts->cancel(); 
                    }
                });
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_mounts_query(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const api::MountsQuery& q = p.mounts_query();
            uuid srcs_of = q.srcs_of().size() > 0 ? boost::lexical_cast<uuid>(q.srcs_of()) : uuid();
            uuid tgts_of = q.tgts_of().size() > 0 ? boost::lexical_cast<uuid>(q.tgts_of()) : uuid();

            query_ptr<mount_info> mounts = ns_->mounts(c, srcs_of, tgts_of);
            if (!mounts) throw remote_error("unable to fetch mounts, got null query");

            api::Packet res;
            api::MountList* m = res.mutable_mount_list();
            for (const auto& i : mounts->current) {
                const auto& mount = i.second;
                api::Mount* mt = m->add_mounts();
                mt->set_src(boost::lexical_cast<std::string>(mount.src));
                mt->set_tgt(boost::lexical_cast<std::string>(mount.tgt));
            }
            conn_.write_back(req_id, std::move(res));
            mounts->added.add(this, [this, req_id] (const mount_info& mount) {
                api::Packet p;
                api::Mount* m = p.mutable_mount_added();
                m->set_src(boost::lexical_cast<std::string>(mount.src));
                m->set_tgt(boost::lexical_cast<std::string>(mount.tgt));
                conn_.write_back(req_id, std::move(p));
            });
            mounts->removed.add(this, [this, req_id] (const mount_info& mount) {
                api::Packet p;
                api::Mount* m = p.mutable_mount_removed();
                m->set_src(boost::lexical_cast<std::string>(mount.src));
                m->set_tgt(boost::lexical_cast<std::string>(mount.tgt));
                conn_.write_back(req_id, std::move(p));
            });
            mounts->cancelled.add(this, [this, req_id] () {
                conn_.close_stream(req_id);
                api::Packet cancel;
                cancel.set_cancel(0);
                conn_.write_back(req_id, std::move(cancel));
            });

            conn_.set_stream_cb(req_id,
                [this, mounts] (io::yield_ctx& yield, const api::Packet& p) {
                    if (p.payload_case() == api::Packet::kCancel) {
                        mounts->cancel(); // will trigger cancelled(), send a response and remove this streamcb
                    }
                });
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_tasks_query(io::yield_ctx& c, const api::Packet& p) {
        try {
            int32_t req_id = p.req_id();
            const api::TasksQuery& q = p.tasks_query();
            uuid u = q.uuid().size() > 0 ? boost::lexical_cast<uuid>(q.uuid()) : uuid();
            const std::string& name = q.name();
            const std::string& type = q.type();

            query_ptr<task_ptr> query = ns_->tasks(c, u, name, type);
            if (!query) throw remote_error("unable to query tasks");

            api::Packet res;
            api::TaskList* l = res.mutable_task_list();
            for (const auto& i : *query) {
                const task_ptr& t = i.second;
                api::Task* task = l->add_tasks();
                std::string uuid = boost::lexical_cast<std::string>(t->get_uuid());
                task->set_uuid(std::move(uuid));
                task->set_name(t->get_name());
                task->set_type(t->get_type());
                api::Params* par = task->mutable_params();
                t->get_params().pack(par);
            }

            query->added.add(this, [this, req_id] (const task_ptr& t) {
                // send back a context added event
                api::Packet p;
                api::Task* task = p.mutable_task_added();
                std::string uuid = boost::lexical_cast<std::string>(t->get_uuid());
                task->set_uuid(std::move(uuid));
                task->set_name(t->get_name());
                task->set_type(t->get_type());
                api::Params* par = task->mutable_params();
                t->get_params().pack(par);

                conn_.write_back(req_id, std::move(p));
            });
            query->removed.add(this, [this, req_id] (const task_ptr& t) {
                std::string uuid = boost::lexical_cast<std::string>(t->get_uuid());
                api::Packet p;
                p.set_task_removed(std::move(uuid));
                conn_.write_back(req_id, std::move(p));
            });

            query->cancelled.add(this, [this, req_id] () {
                conn_.close_stream(req_id);
                api::Packet cancel;
                cancel.set_cancel(0);
                conn_.write_back(req_id, std::move(cancel));
            });
            conn_.set_stream_cb(req_id,
                [this, query] (io::yield_ctx& yield, const api::Packet& p) {
                    if (p.payload_case() == api::Packet::kCancel) {
                        // will trigger cancelled(), which will write back a cancel packet
                        // before closing the stream and removing the captured query ptr
                        query->cancel(); 
                    }
                });
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
