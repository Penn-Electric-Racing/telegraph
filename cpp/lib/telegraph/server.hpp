#ifndef __TELEGRAPH_SERVER_HPP__
#define __TELEGRAPH_SERVER_HPP__

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
#include "nodes/variable.hpp"

#include "context.hpp"

#include "api.grpc.pb.h"
#include "api.pb.h"


namespace telegraph {

    class id_manager {
    public:
        id_manager();

        void add_context(context* ctx);
        void remove_context(context* ctx);

        void add_node(context* ctx, node* n);
        void remove_node(context* ctx, node* n);

        inline int32_t get_id(context* ctx) const { 
            try {
                return ctx_id_map_.at(ctx);
            } catch (std::out_of_range& e) { return -1; }
        }
        inline int32_t get_id(node* n) const { 
            try {
                return node_id_map_.at(n);
            } catch (std::out_of_range& e) { return -1; }
        }
        inline node* get_node(int32_t id) const { 
            try {
                return id_node_map_.at(id);
            } catch (std::out_of_range& e) { return nullptr; }
        }
        inline std::unordered_set<int32_t> get_nodes(context *ctx) const {
            try {
                return ctx_node_ids_map_.at(ctx);
            } catch(std::out_of_range& e) {
                return std::unordered_set<int32_t>();
            }
        }
        inline context* get_context(int32_t id) const { 
            try {
                return id_ctx_map_.at(id);
            } catch (std::out_of_range& e) { return nullptr; }
        }
        inline context* get_context_from_node(int32_t nid) const {
            try {
                return node_id_ctx_map_.at(nid);
            } catch (std::out_of_range& e) { return nullptr; }
        }
    private:
        int32_t ctx_id_counter_;
        std::unordered_map<int32_t, context*> id_ctx_map_;
        std::unordered_map<context*, int32_t> ctx_id_map_;

        int32_t node_id_counter_;
        std::unordered_map<int32_t, node*> id_node_map_;
        std::unordered_map<node*, int32_t> node_id_map_;

        std::unordered_map<context*, std::unordered_set<int32_t>> ctx_node_ids_map_;
        std::unordered_map<int32_t, context*> node_id_ctx_map_;
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
            request(request_type t, server* srv) : type(t), status(CREATED), srv(srv), alarm_() {}
            inline virtual ~request() {}

            // create a new identical, request handler
            virtual request* create() = 0;

            virtual void reg() = 0;

            // when next is called for this request type
            virtual void called() = 0; 
            virtual void done() = 0;

            inline void mark_finished() { status = FINISHED; }

            inline void alarm() {
                alarm_.Set(srv->cq_.get(), gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME), this);
            }

            // these need to be modified by other people
            // and setters/getters are kinda extreme
            request_type type;
            request_status status;
            server* srv;
        private:
            grpc::Alarm alarm_;
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
        public:
            inline tree_request(server* srv) : request(request_type::STREAM_TREE, srv),
                            req_mutex_(), ctx_(), request_(), responder_(&ctx_), 
                            tree_(), queue_() {}
            inline request* create() override { return new tree_request(srv); }

            void reg() override;

            void called() override;
            void done() override;
        private:
            std::mutex req_mutex_;

            grpc::ServerContext ctx_;
            proto::TreeRequest request_;
            grpc::ServerAsyncWriter<proto::TreeDelta> responder_;

            context::handle tree_;

            std::queue<proto::TreeDelta> queue_;
        };

        class subscribe_request : public request {
        public:
            inline subscribe_request(server* srv) :
                request(request_type::SUBSCRIBE, srv), req_mutex_(),
                ctx_(), request_(), responder_(&ctx_), queue_() {}
            inline request* create() override { return new subscribe_request(srv); }

            void reg() override;

            void called() override;
            void done() override;
        private:
            std::mutex req_mutex_;

            grpc::ServerContext ctx_;
            proto::SubscribeRequest request_;
            grpc::ServerAsyncWriter<proto::DataPoint> responder_;

            variable* variable_;
            std::shared_ptr<variable::subscription> subscription_;

            std::queue<proto::DataPoint> queue_;
        };

        class action_request : public request {
        public:
            inline action_request(server* srv) :
                request(request_type::PERFORM_ACTION, srv), req_mutex_(),
                ctx_(), request_(), responder_(&ctx_) {}
            inline request* create() override { return new action_request(srv); }

            void reg() override;

            void called() override;
            void done() override;
        private:
            std::mutex req_mutex_;

            grpc::ServerContext ctx_;
            proto::ActionRequest request_;
            grpc::ServerAsyncResponseWriter<proto::Value> responder_;
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
