#ifndef __PER_SERVER_HPP__
#define __PER_SERVER_HPP__

#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "api.grpc.pb.h"
#include "api.pb.h"

namespace per {
    class context;

    class server {
    public:
        server();
        ~server();

        void start(const std::string& bind);
        void stop();
        void join();

        void add_context(context* ctx);
        void remove_context(context* ctx);

    private:
        enum request_type { STREAM_CONTEXTS, STREAM_TREE, 
                            SUBSCRIBE, PERFORM_ACTION };
        enum request_status { CREATED, RUNNING, FINISHED };

        // a request handler class
        class request {
        public:
            constexpr request(request_type t) : type(t), status(CREATED) {}
            inline virtual ~request() {}

            // create a new identical, request handler
            virtual request* create() = 0;

            virtual void reg(proto::ContextManager::AsyncService* s,
                             grpc::ServerCompletionQueue* cq) = 0;

            // when next is called for this request type
            virtual void called(server* s) = 0; 
            virtual void done(server* s) = 0;

            inline void mark_finished() { status = FINISHED; }

            request_type type;
            request_status status;
        };

        class contexts_request : public request {
        public:
            inline contexts_request() : 
                request(request_type::STREAM_CONTEXTS),
                responder_(&ctx_),
                queue_mutex_(), queue_() {}

            inline request* create() override { return new contexts_request(); }

            void enqueue(server* s, const proto::ContextDelta& d);

            void reg(proto::ContextManager::AsyncService* s,
                             grpc::ServerCompletionQueue* cq) override;

            void called(server* s) override;
            void done(server* s) override;
        private:
            grpc::ServerContext ctx_;
            proto::Empty request_;
            grpc::ServerAsyncWriter<proto::ContextDelta> responder_;

            // the queue of context deltas left to send
            std::mutex queue_mutex_;
            std::queue<proto::ContextDelta> queue_;
        };

        friend class contexts_request;

        void loop();

        proto::ContextManager::AsyncService service_;
        std::unique_ptr<grpc::Server> server_;
        std::unique_ptr<grpc::ServerCompletionQueue> cq_;

        std::thread thread_;

        // mutex for mutating/accessing 
        // the set of open context requests
        std::mutex ctx_mutex_;

        // any open context streams
        std::unordered_set<contexts_request*> ctx_streams_;

        size_t ctx_id_counter_;
        std::unordered_map<size_t, context*> id_ctx_map_;
        std::unordered_map<context*, size_t> ctx_id_map_;
    };
}

#endif
