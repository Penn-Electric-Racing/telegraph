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
    }

    void
    forwarder::handle_call_action(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_data_write(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_data_query(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_start_task(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_stop_task(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_mount(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_unmount(io::yield_ctx& c, const api::Packet& p) {
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
                c->set_ns(boost::lexical_cast<std::string>(ns_->get_uuid()));
                std::string uuid = boost::lexical_cast<std::string>(ctx->get_uuid());
                c->set_uuid(std::move(uuid));
                c->set_name(ctx->get_name());
                c->set_type(ctx->get_type());
                api::Info* info = c->mutable_info();
                ctx->get_info().pack(info);
            }
            conn_.write_back(p.req_id(), std::move(res));

            // add handlers to the query
            int32_t req_id = p.req_id();
            contexts->added.add(this, [this, req_id] (io::yield_ctx& yield, 
                                                        const context_ptr& ctx) {
                // send back a context added event
                api::Packet p;
                api::Context* c = p.mutable_context_added();
                c->set_ns(boost::lexical_cast<std::string>(ns_->get_uuid()));
                std::string uuid = boost::lexical_cast<std::string>(ctx->get_uuid());
                c->set_uuid(std::move(uuid));
                c->set_name(ctx->get_name());
                c->set_type(ctx->get_type());
                api::Info* info = c->mutable_info();
                ctx->get_info().pack(info);

                conn_.write_back(req_id, std::move(p));
            });
            contexts->removed.add(this, [this, req_id] (io::yield_ctx& yield, 
                                                           const context_ptr& ctx) {
                std::string uuid = boost::lexical_cast<std::string>(ctx->get_uuid());
                api::Packet p;
                p.set_context_removed(std::move(uuid));
                conn_.write_back(req_id, std::move(p));
            });

            // set stream handler to wait for cancel
            // this will own the query, so when
            // the connection is destructed or the stream
            // cancelled, the query will also be destroyed
            conn_.set_stream_cb(req_id,
                [this, contexts] (io::yield_ctx& yield, const api::Packet& p) {
                    if (p.has_cancel()) {
                        api::Packet success;
                        success.set_success(true);
                        conn_.write_back(p.req_id(), std::move(success));
                        conn_.close_stream(p.req_id()); // will delete handler
                    }
                });
        } catch (const error& e) {
            reply_error(p, e);
        }
    }

    void
    forwarder::handle_mounts_query(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_tasks_query(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_create_context(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_create_task(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_destroy_context(io::yield_ctx& c, const api::Packet& p) {
    }

    void
    forwarder::handle_destroy_task(io::yield_ctx& c, const api::Packet& p) {
    }
}
