#include "namespace.hpp"
#include "../common/nodes.hpp"
#include "../utils/errors.hpp"
#include "../utils/uuid.hpp"

#include "connection.hpp"
#include "api.pb.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace telegraph {
    remote_namespace::remote_namespace(connection& conn)
                        : namespace_(uuid()), 
                          conn_(conn) {
    }

    remote_namespace::~remote_namespace() {
        // cleanup...
    }

    void
    remote_namespace::connect(io::yield_ctx& yield) {
        api::Packet req;
        req.mutable_query_ns(); // set to use query_ns
        api::Packet res = conn_.request_response(yield, std::move(req));
        if (res.payload_case() != api::Packet::kNsUuid) {
            throw remote_error("did not errors");
        }
        uuid_ = boost::lexical_cast<uuid>(res.ns_uuid());
    }

    bool
    remote_namespace::is_connected() const {
        return get_uuid().is_nil();
    }

    query_ptr<mount_info>
    remote_namespace::mounts(io::yield_ctx&,
            const uuid& srcs_of, const uuid& tgts_of) const {
        return nullptr;
    }

    query_ptr<context_ptr>
    remote_namespace::contexts(io::yield_ctx& yield,
            const uuid& by_uuid, const std::string& by_name, 
            const std::string& by_type) const {
        return nullptr;
    }

    query_ptr<task_ptr>
    remote_namespace::tasks(io::yield_ctx& yield,
            const uuid& by_uuid, const std::string& by_name,
            const std::string& by_type) const {
        return nullptr;
    }

    std::shared_ptr<node>
    remote_namespace::fetch(io::yield_ctx& yield,
                        const uuid& uuid, context_ptr owner) const {
        return nullptr;
    }

    subscription_ptr
    remote_namespace::subscribe(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path,
            interval min_interval, interval max_interval, interval timeout) {
        return nullptr;
    }

    value
    remote_namespace::call(io::yield_ctx& yield,
                const uuid& ctx, 
                const std::vector<std::string>& path, value arg, interval timeout) {
        return value();
    }

    std::unique_ptr<data_query>
    remote_namespace::query_data(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path) const {
        return nullptr;
    }

    bool
    remote_namespace::write_data(io::yield_ctx& yield,
            const uuid& ctx, const std::vector<std::string>& path,
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
}
