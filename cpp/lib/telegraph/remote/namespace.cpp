#include "namespace.hpp"
#include "../common/nodes.hpp"
#include "../utils/errors.hpp"
#include "../utils/uuid.hpp"

#include "connection.hpp"
#include "api.pb.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace telegraph {
    remote_namespace::remote_namespace(io::io_context& ioc, connection& conn)
                        : namespace_(uuid()), ioc_(ioc), conn_(conn) {
    }

    remote_namespace::~remote_namespace() {
        // cleanup...
    }

    void
    remote_namespace::connect(io::yield_ctx& yield, const std::weak_ptr<remote_namespace>& wptr) {
        wptr_ = wptr;

        api::Packet req;
        req.mutable_query_ns(); // set to use query_ns
        api::Packet res = conn_.request_response(yield, std::move(req));
        if (res.payload_case() != api::Packet::kNsUuid) {
            throw remote_error("did not get ns_uuid response");
        }
        uuid_ = boost::lexical_cast<uuid>(res.ns_uuid());
    }

    bool
    remote_namespace::is_connected() const {
        return get_uuid().is_nil();
    }

    context_ptr
    remote_namespace::create_context(io::yield_ctx& yield,
            const std::string_view& name, const std::string_view& type,
            const info& params, const sources_map& srcs) {
        api::Packet cp;

        api::Packet res = conn_.request_response(yield, std::move(cp));
        if (res.context_created().size() == 0)
            throw io_error("expected context_created");
        uuid ctx_uuid = boost::lexical_cast<uuid>(res.context_created());

        // send create request
        query_ptr<context_ptr> q = contexts(yield, ctx_uuid);
        if (!q)
            throw io_error("unable to query contexts");
        return q->result();
    }

    void
    remote_namespace::destroy_context(io::yield_ctx&, const uuid& u) {
    }

    task_ptr
    remote_namespace::create_task(io::yield_ctx& yield,
            const std::string_view& name, const std::string_view& type,
            const info& params, const sources_map& srcs) {
        return nullptr;
    }

    void
    remote_namespace::destroy_task(io::yield_ctx&, const uuid& u) {
    }

    query_ptr<mount_info>
    remote_namespace::mounts(io::yield_ctx&,
            const uuid& srcs_of, const uuid& tgts_of) const {
        return nullptr;
    }

    query_ptr<context_ptr>
    remote_namespace::contexts(io::yield_ctx& yield,
            const uuid& by_uuid, const std::string_view& by_name, 
            const std::string_view& by_type) const {
        api::Packet qp;
        api::ContextsQuery* cq = qp.mutable_contexts_query();

        if (!by_uuid.is_nil()) cq->set_uuid(boost::lexical_cast<std::string>(by_uuid));
        if (by_name.size() > 0) {
            std::string n{by_name};
            cq->set_name(std::move(n));
        }
        if (by_type.size() > 0) {
            std::string t{by_type};
            cq->set_type(std::move(t));
        }

        // make the query object
        query_ptr<context_ptr> q = std::make_shared<query<context_ptr>>();

        std::weak_ptr<query<context_ptr>> wq{q};
        api::Packet res = conn_.request_stream(yield, std::move(qp),
            [wq, this] (io::yield_ctx& yield, const api::Packet& p) {
                query_ptr<context_ptr> q = wq.lock();
                if (!q) return;
                if (p.has_context_added()) {
                    auto s = wptr_.lock(); // get ptr to this namespace
                    const api::Context& c = p.context_added();
                    uuid u = boost::lexical_cast<uuid>(c.uuid());
                    context_ptr ctx = 
                        std::make_shared<remote_context>(ioc_, s, u,
                                                c.name(), c.type(), info{c.info()});
                    q->add_(yield, ctx);
                } else if (p.context_removed().size() > 0) {
                    uuid u = boost::lexical_cast<uuid>(p.context_removed());
                    context_ptr p = q->get(u);
                    q->remove_by_key_(yield, u);
                    p->destroy(yield);
                }
            });

        int32_t req_id = res.req_id();

        // from the response populate the query
        if (!res.has_context_list()) {
            throw remote_error("did not receive context list in response");
        }

        // get shared ptr to this remote namespace
        // so as long as the query/contexts are alive, the namespace
        // remains alive
        auto s = wptr_.lock();

        const api::ContextList& l = res.context_list();
        for (int i = 0; i < l.contexts_size(); i++) {
            const api::Context& c = l.contexts(i);
            uuid u = boost::lexical_cast<uuid>(c.uuid());
            context_ptr ctx = 
                std::make_shared<remote_context>(ioc_, s, u,
                                        c.name(), c.type(), info{c.info()});
            q->add_(yield, ctx);
        }

        // tie query cancelled handler to a shared
        // ptr to this namespace
        q->cancelled.add([s, req_id] () {
                connection& c = s->get_conn();
                // write cancel message
                // and close the stream (destroying the shared ptr reference
                // to this namespace)
                api::Packet cancel;
                cancel.set_cancel(0); // cancel doesn't matter since this is a query--will be instantenous
                c.write_back(req_id, std::move(cancel));
                c.close_stream(req_id);
            });
        return q;
    }

    query_ptr<task_ptr>
    remote_namespace::tasks(io::yield_ctx& yield,
            const uuid& by_uuid, const std::string_view& by_name,
            const std::string_view& by_type) const {
        return nullptr;
    }

    std::shared_ptr<node>
    remote_namespace::fetch(io::yield_ctx& yield,
                        const uuid& uuid, context_ptr owner) const {
        return nullptr;
    }

    subscription_ptr
    remote_namespace::subscribe(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string_view>& path,
            float min_interval, float max_interval, float timeout) {
        return nullptr;
    }

    value
    remote_namespace::call(io::yield_ctx& yield,
                            const uuid& ctx, const std::vector<std::string_view>& path, 
                            value arg, float timeout) {
        return value();
    }

    std::unique_ptr<data_query>
    remote_namespace::query_data(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string_view>& path) const {
        return nullptr;
    }

    bool
    remote_namespace::write_data(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string_view>& path,
            const std::vector<data_point>& data) {
        return false;
    }

    void
    remote_namespace::mount(io::yield_ctx& yield,
            const uuid& src, const uuid& tgt) {
    }

    void
    remote_namespace::unmount(io::yield_ctx& yield,
            const uuid& src, const uuid& tgt) {
    }
    

    void
    remote_namespace::start_task(io::yield_ctx& yield, const uuid& task) {

    }

    void
    remote_namespace::stop_task(io::yield_ctx& yield, const uuid& task) {

    }

    info_stream_ptr
    remote_namespace::query_task(io::yield_ctx& yield, const uuid& task, const info& i) {
        return nullptr;
    }


    // remote_context functions

    std::shared_ptr<node>
    remote_context::fetch(io::yield_ctx& yield) {
        return ns_->fetch(yield, get_uuid(), shared_from_this());
    }

    subscription_ptr
    remote_context::subscribe(io::yield_ctx& ctx, variable* v,
                            float min_interval, float max_interval,
                            float timeout) {
        return nullptr;
    }

    subscription_ptr
    remote_context::subscribe(io::yield_ctx& ctx, const std::vector<std::string_view>& variable,
                            float min_interval, float max_interval,
                            float timeout) {
        return nullptr;
    }

    value
    remote_context::call(io::yield_ctx& ctx, action* a, value v, float timeout) {
        return value();
    }

    value
    remote_context::call(io::yield_ctx& ctx, const std::vector<std::string_view>& a,
                            value v, float timeout) {
        return value();
    }

    bool
    remote_context::write_data(io::yield_ctx& yield, variable* v,
                              const std::vector<data_point>& data) {
        return false;
    }
    bool
    remote_context::write_data(io::yield_ctx& yield, const std::vector<std::string_view>& v,
                              const std::vector<data_point>& data) {
        return false;
    }

    data_query_ptr
    remote_context::query_data(io::yield_ctx& yield, const node* n) const {
        return nullptr;
    }
    data_query_ptr
    remote_context::query_data(io::yield_ctx& yield, const std::vector<std::string_view>& n) const {
        return nullptr;
    }

    query_ptr<mount_info>
    remote_context::mounts(io::yield_ctx& yield, bool srcs, bool tgts) const {
        return nullptr;
    }

    void
    remote_context::mount(io::yield_ctx& ctx, const context_ptr& src) {
    }

    void
    remote_context::unmount(io::yield_ctx& ctx, const context_ptr& src) {
    }

    void
    remote_context::destroy(io::yield_ctx& yield) {
        context::destroy(yield);
    }
}
