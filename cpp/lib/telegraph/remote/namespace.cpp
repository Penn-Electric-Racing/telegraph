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
                        : namespace_(), ioc_(ioc), conn_(conn) {
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
        if (res.payload_case() != api::Packet::kNs) {
            throw remote_error("did not get namespace response!");
        }
        // unpack the contexts/components/mounts

        // add listeners for add/remove of contexts/components/mounts
    }

    bool
    remote_namespace::is_connected() const {
        return wptr_.lock() == nullptr;
    }

    context_ptr
    remote_namespace::create_context(io::yield_ctx& yield,
            const std::string_view& name, const std::string_view& type,
            const params& p, sources_uuid_map&& srcs) {
        api::Packet cp;

        api::Packet res = conn_.request_response(yield, std::move(cp));
        if (res.context_created().size() == 0)
            throw io_error("expected context_created");
        uuid ctx_uuid = boost::lexical_cast<uuid>(res.context_created());
        return contexts->get(ctx_uuid);
    }

    void
    remote_namespace::destroy_context(io::yield_ctx&, const uuid& u) {
    }

    component_ptr
    remote_namespace::create_component(io::yield_ctx& yield,
            const std::string_view& name, const std::string_view& type,
            const params& p, sources_uuid_map&& srcs) {
        return nullptr;
    }

    void
    remote_namespace::destroy_component(io::yield_ctx&, const uuid& u) {
    }

    // remote_context functions

    collection_ptr<mount_info>
    remote_context::mounts(bool srcs, bool tgts) const {
        const uuid& u = uuid_;
        return ns_->mounts->filter([u, srcs, tgts](const mount_info& i) {
            auto s = i.src.lock();
            auto t = i.tgt.lock();
            if (!s || !t) return false;
            return (srcs && (t->get_uuid() == u)) || (tgts && (s->get_uuid() == u));
        });
    }

    std::shared_ptr<node>
    remote_context::fetch(io::yield_ctx& yield) {
        return nullptr;
    }

    subscription_ptr
    remote_context::subscribe(io::yield_ctx& ctx, const std::vector<std::string_view>& variable,
                            float min_interval, float max_interval,
                            float timeout) {
        return nullptr;
    }

    value
    remote_context::call(io::yield_ctx& ctx, const std::vector<std::string_view>& a,
                            value v, float timeout) {
        return value();
    }

    bool
    remote_context::write_data(io::yield_ctx& yield, const std::vector<std::string_view>& v,
                              const std::vector<data_point>& data) {
        return false;
    }

    data_query_ptr
    remote_context::query_data(io::yield_ctx& yield, const std::vector<std::string_view>& n) {
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
        // should trigger destroyed() when the remote context is
        // removed from its parent query
        ns_->destroy_context(yield, get_uuid());
    }
}
