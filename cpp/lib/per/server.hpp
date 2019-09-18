#ifndef __PER_SERVER_HPP__
#define __PER_SERVER_HPP__

#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>

#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>

#include "utils/signal.hpp"

#include "nodes/tree.hpp"

#include "context.hpp"

#include "api.grpc.pb.h"
#include "api.pb.h"


namespace per {

    class id_manager {
    public:
        id_manager();

        void add_context(context* ctx);
        void remove_context(context* ctx);

        void add_node(context* ctx, node* n);
        void remove_node(context* ctx, node* n);

        inline size_t get_id(context* ctx) const { 
            try {
                return ctx_id_map_.at(ctx);
            } catch (std::out_of_range& e) { return 0; }
        }
        inline node* get_node(size_t id) const { 
            try {
                return id_node_map_.at(id);
            } catch (std::out_of_range& e) { return nullptr; }
        }
    private:
        size_t ctx_id_counter_;
        std::unordered_map<size_t, context*> id_ctx_map_;
        std::unordered_map<context*, size_t> ctx_id_map_;

        size_t node_id_counter_;
        std::unordered_map<size_t, node*> id_node_map_;
        std::unordered_map<node*, size_t> node_id_map_;
        std::unordered_map<context*, std::unordered_set<size_t>> ctx_node_ids_;
    };

    /**
     * For starting/stopping a server
     * as well as adding contexts
     */
    class server {
    public:
        server();
        ~server();

        void start(const std::string& bind);
        void stop();
        void join();

        context_set contexts;
    private:
        enum request_type { STREAM_CONTEXTS, STREAM_TREE, 
                            SUBSCRIBE, PERFORM_ACTION };
        enum request_status { CREATED, RUNNING, FINISHED };

        // a request handler class
        class request {
        public:
            constexpr request(request_type t, server* srv) : type(t), status(CREATED), srv(srv) {}
            inline virtual ~request() {}

            // create a new identical, request handler
            virtual request* create() = 0;

            virtual void reg() = 0;

            // when next is called for this request type
            virtual void called() = 0; 
            virtual void done() = 0;

            inline void mark_finished() { status = FINISHED; }

            inline void alarm() {
                grpc::Alarm alarm;
                alarm.Set(srv->cq_.get(), gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME), this);
            }

            // these need to be modified by other people
            // and setters/getters are kinda extreme
            request_type type;
            request_status status;
            server* srv;
        };

        class contexts_request : public request {
        public:
            inline contexts_request(server* srv) : 
                request(request_type::STREAM_CONTEXTS, srv),
                ctx_(), request_(),
                responder_(&ctx_),
                queue_mutex_(), queue_() {}

            inline request* create() override { return new contexts_request(srv); }

            void enqueue(const proto::ContextDelta& d);

            void reg() override;

            void called() override;
            void done() override;
        private:
            grpc::ServerContext ctx_;
            proto::Empty request_;
            grpc::ServerAsyncWriter<proto::ContextDelta> responder_;

            // the queue of context deltas left to send
            std::mutex queue_mutex_;
            std::queue<proto::ContextDelta> queue_;
        };

        class tree_request : public request {
            inline tree_request(server* srv) : request(request_type::STREAM_TREE, srv),
                            ctx_(), request_(),
                            responder_(&ctx_), queue_mutex_(), queue_() {}
            inline request* create() override { return new tree_request(srv); }

            void reg() override;

            void called() override;
            void done() override;
        private:
            grpc::ServerContext ctx_;
            proto::TreeRequest request_;
            grpc::ServerAsyncWriter<proto::TreeDelta> responder_;

            tree* tree_;

            std::mutex queue_mutex_;
            std::queue<proto::TreeDelta> queue_;
        };

        friend class contexts_request;

        void loop();

        proto::ContextManager::AsyncService service_;
        std::unique_ptr<grpc::Server> server_;
        std::unique_ptr<grpc::ServerCompletionQueue> cq_;

        std::thread thread_;
        mutable std::recursive_mutex ids_mutex_;
        id_manager ids_;
    };
}

#endif
