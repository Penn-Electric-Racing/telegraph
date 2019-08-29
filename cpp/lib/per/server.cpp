#include "server.hpp"

#include "context.hpp"

#include <iostream>

#include <grpcpp/alarm.h>


namespace per {

    server::server() : service_(), server_(), cq_() {}
    server::~server() {
        stop();
    }

    void
    server::start(const std::string& bind) {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(bind, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();

        thread_ = std::thread(&server::loop, this);
    }

    void
    server::stop() {
    }

    void
    server::join() {
        thread_.join();
    }

    void
    server::add_context(context* ctx) {
        std::lock_guard<std::mutex> m(ctx_mutex_);
        if (ctx_id_map_.find(ctx) != ctx_id_map_.end()) return;

        size_t id = ctx_id_counter_++;
        id_ctx_map_[id] = ctx;
        ctx_id_map_[ctx] = id;

        ctx->on_dispose.add(this, [ctx, this] () { remove_context(ctx); });

        proto::ContextDelta d;
        d.set_type(proto::DeltaType::ADDED);
        d.mutable_context()->set_id(id);
        d.mutable_context()->set_name(ctx->get_name());

        for (contexts_request* r : ctx_streams_) {
            r->enqueue(this, d);
        }
    }

    void
    server::remove_context(context* ctx) {
        std::lock_guard<std::mutex> m(ctx_mutex_);
        auto it = ctx_id_map_.find(ctx);
        if (it == ctx_id_map_.end()) return;
        size_t id = it->second;

        ctx->on_dispose.remove(this);

        proto::ContextDelta d;
        d.set_type(proto::DeltaType::REMOVED);
        d.mutable_context()->set_id(id);
        d.mutable_context()->set_name(ctx->get_name());

        for (contexts_request* r : ctx_streams_) {
            r->enqueue(this, d);
        }
    }

    void
    server::loop() {
        std::unordered_set<request*> open_requests;
        open_requests.insert(new contexts_request());

        // register all requests
        for (request* r : open_requests) {
            r->reg(&service_, cq_.get());
        }

        void* tag;
        bool ok;
        while (true) {
            cq_->Next(&tag, &ok);
            if (!ok) break;
            request* r = (request*) tag;
            if (r->status == CREATED) {
                request* n = r->create();
                n->reg(&service_, cq_.get());
                open_requests.insert(n);
                r->called(this);
                r->status = RUNNING;
            } else if (r->status == RUNNING) {
                r->called(this);
            } else if (r->status == FINISHED) {
                r->done(this);
                open_requests.erase(r);
                delete r;
            }
        }

        for (request* r : open_requests) {
            if (r->status != CREATED) r->done(this);
            delete r;
        }

        // this better be empty!
        // (we can do this safely since at this point
        // nobody should be writing to ctx_streams_)
        assert(ctx_streams_.size() == 0);
    }

    // ---------- streaming context request ---------

    void
    server::contexts_request::reg(proto::ContextManager::AsyncService* s,
                                 grpc::ServerCompletionQueue* cq) {
        s->RequestStreamContexts(&ctx_, &request_, &responder_, cq, cq, this);
    }

    void
    server::contexts_request::enqueue(server* s, const proto::ContextDelta& d) {
        std::lock_guard<std::mutex> guard(queue_mutex_);
        queue_.push(d);
        grpc::Alarm alarm;
        alarm.Set(s->cq_.get(), gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME), this);
    }

    void
    server::contexts_request::called(server* s) {
        if (status == CREATED) { // first time called! queue existing contexts
            std::lock_guard<std::mutex> guard(s->ctx_mutex_);
            for (auto& p : s->id_ctx_map_) {
                proto::ContextDelta delta;
                delta.set_type(proto::DeltaType::INITIAL);
                delta.mutable_context()->set_id(p.first);
                delta.mutable_context()->set_name(p.second->get_name());
                queue_.push(delta);
            }
            proto::ContextDelta delta;
            delta.set_type(proto::DeltaType::INITIALIZED);
            queue_.push(delta);
        }
        // write a single context
        std::lock_guard<std::mutex> guard(queue_mutex_);
        if (!queue_.empty()) {
            const proto::ContextDelta& delta = queue_.front();
            responder_.Write(delta, this);
            queue_.pop();
        }
    }

    void
    server::contexts_request::done(server* s) {
        s->ctx_streams_.erase(this);
    }
}
