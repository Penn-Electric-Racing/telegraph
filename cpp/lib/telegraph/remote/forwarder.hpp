#ifndef __TELEGRAPH_FORWARDER_HPP__
#define __TELEGRAPH_FORWARDER_HPP__

#include <memory>
#include <unordered_set>

#include "api.pb.h"
#include "../utils/io_fwd.hpp"
#include "../utils/errors.hpp"

#include "../common/collection.hpp"
#include "../common/data.hpp"
#include "../common/namespace.hpp"

#include <unordered_map>

namespace telegraph {
    class connection;
    class namespace_;

    // will take a connection and a local namespace
    // and forward incoming requests to that local namespace
    class forwarder {
    private:
        connection& conn_;
        std::shared_ptr<namespace_> ns_;
        // active subscriptions
        std::unordered_map<int32_t, subscription_ptr> subs_;
        // active component query streams
        std::unordered_map<int32_t, params_stream_ptr> streams_;
        std::unordered_map<int32_t, data_query_ptr> queries_;
    public:
        // will register handlers
        forwarder(connection& conn, 
                const std::shared_ptr<namespace_>& ns);
        ~forwarder();
    private:
        void reply_error(const api::Packet& p, const std::exception& e);

        void handle_query_ns(io::yield_ctx&, const api::Packet& p);

        void handle_stream_context(io::yield_ctx&, const api::Packet& p);
        void handle_stream_component(io::yield_ctx&, const api::Packet& p);

        void handle_fetch_tree(io::yield_ctx&, const api::Packet& p);

        void handle_sub_change(io::yield_ctx&, const api::Packet& p);
        void handle_call_action(io::yield_ctx&, const api::Packet& p);

        void handle_data_write(io::yield_ctx&, const api::Packet& p);
        void handle_data_query(io::yield_ctx&, const api::Packet& p);

        void handle_mount(io::yield_ctx&, const api::Packet& p);
        void handle_unmount(io::yield_ctx&, const api::Packet& p);

        void handle_create_context(io::yield_ctx&, const api::Packet& p);
        void handle_create_component(io::yield_ctx&, const api::Packet& p);
        void handle_destroy_context(io::yield_ctx&, const api::Packet& p);
        void handle_destroy_component(io::yield_ctx&, const api::Packet& p);
    };
}

#endif
