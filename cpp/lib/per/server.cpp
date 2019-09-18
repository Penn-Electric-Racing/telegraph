#include "server.hpp"

#include "context.hpp"

#include <iostream>

namespace per {

    id_manager::id_manager() : ctx_id_counter_(0),
                               id_ctx_map_(), ctx_id_map_(),
                               node_id_counter_(0), 
                               id_node_map_(), node_id_map_(),
                               ctx_node_ids_() {}

    void
    id_manager::add_context(context* ctx) {
    }
    void
    id_manager::remove_context(context* ctx) {
        try {
            size_t cid = ctx_id_map_.at(ctx);
            id_ctx_map_.erase(cid);
            ctx_id_map_.erase(ctx);
        } catch (std::out_of_range& e) {
            return;
        }
        try {
            std::unordered_set<size_t>& v = ctx_node_ids_.at(ctx);
            for (size_t i : v) {
                try {
                    node* n = id_node_map_.at(i);
                    node_id_map_.erase(n);
                    id_node_map_.erase(i);
                } catch (std::out_of_range& e) {}
            }
            ctx_node_ids_.erase(ctx);
        } catch (std::out_of_range& e) {}
    }
    void
    id_manager::add_node(context* ctx, node* n) {
        if (node_id_map_.find(n) != node_id_map_.end()) return;
        size_t id = node_id_counter_++;
        node_id_map_[n] = id;
        id_node_map_[id] = n;
        ctx_node_ids_[ctx].insert(id);
    }
    void
    id_manager::remove_node(context* ctx, node* n) {
        try { 
            size_t id = node_id_map_.at(n);
            ctx_node_ids_[ctx].erase(id);
        } catch (std::out_of_range& e) {}
    }

    server::server() : 
        contexts(),
        service_(), server_(), cq_(), thread_(),
        ids_() {
        // add listeners to the context set
        contexts.context_added.add(this,
            [this] (context* ctx) {
                std::lock_guard<std::recursive_mutex> guard(ids_mutex_);
                ids_.add_context(ctx);

                auto cl = ctx->lock();
                if (ctx->get_tree()) {
                    for (node* n : ctx->get_tree()->nodes()) {
                        ids_.add_node(ctx, n);
                    }
                }
                ctx->on_tree_change.add(this, [ctx, this](tree* t) {
                    std::lock_guard<std::recursive_mutex> guard(ids_mutex_);
                    auto cl = ctx->lock();
                    for (node* n : t->nodes()) {
                        ids_.add_node(ctx, n);
                    }
                });
            });
        contexts.context_removed.add(this,
            [this] (context* ctx) {
                std::lock_guard<std::recursive_mutex> guard(ids_mutex_);
                auto cl = ctx->lock();
                ids_.remove_context(ctx);
                ctx->on_tree_change.remove(this);
            });
    }

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
    server::loop() {
        std::unordered_set<request*> open_requests;
        open_requests.insert(new contexts_request(this));
        // register all requests
        for (request* r : open_requests) r->reg();

        void* tag;
        bool ok;
        while (true) {
            cq_->Next(&tag, &ok);
            if (!ok) break;
            request* r = (request*) tag;
            if (r->status == CREATED) {
                request* n = r->create();
                n->reg();
                open_requests.insert(n);
                r->called();
                r->status = RUNNING;
            } else if (r->status == RUNNING) {
                r->called();
            } else if (r->status == FINISHED) {
                r->done();
                open_requests.erase(r);
                delete r;
            }
        }

        for (request* r : open_requests) {
            if (r->status != CREATED) r->done();
            delete r;
        }
    }

    // ---------- streaming context request ---------

    void
    server::contexts_request::reg() {
        auto cq = srv->cq_.get();
        srv->service_.RequestStreamContexts(&ctx_, &request_, &responder_, cq, cq, this);
    }

    void
    server::contexts_request::enqueue(const proto::ContextDelta& d) {
        std::lock_guard<std::mutex> guard(queue_mutex_);
        queue_.push(d);
        alarm();
    }

    void
    server::contexts_request::called() {
        if (status == CREATED) { // first time called! queue existing contexts
                                 // and adds listeners
            auto& contexts = srv->contexts;

            // lock the contexts
            auto cl = contexts.lock();
            std::lock_guard<std::recursive_mutex> guard(srv->ids_mutex_);
            for (context* ctx : contexts.set()) {
                auto l = ctx->lock();
                size_t id = srv->ids_.get_id(ctx);
                proto::ContextDelta delta;
                delta.set_type(proto::DeltaType::INITIAL);
                delta.mutable_context()->set_id(id);
                delta.mutable_context()->set_name(ctx->get_name());
                queue_.push(delta);
            }

            proto::ContextDelta delta;
            delta.set_type(proto::DeltaType::INITIALIZED);
            queue_.push(delta);

            contexts.context_added.add(this, [this](context* ctx) {
                    proto::ContextDelta delta;
                    delta.set_type(proto::DeltaType::ADDED);
                    {
                        // lock the ids and the context
                        std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                        auto l = ctx->lock();
                        size_t id = srv->ids_.get_id(ctx);
                        delta.mutable_context()->set_id(id);
                        delta.mutable_context()->set_name(ctx->get_name());
                    }

                    std::lock_guard<std::mutex> guard(queue_mutex_);
                    queue_.push(delta);
                });
            contexts.context_removed.add(this, [this](context* ctx) {
                    proto::ContextDelta delta;
                    delta.set_type(proto::DeltaType::REMOVED);
                    {
                        std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                        auto l = ctx->lock();
                        size_t id = srv->ids_.get_id(ctx);
                        delta.mutable_context()->set_id(id);
                        delta.mutable_context()->set_name(ctx->get_name());
                    }
                    std::lock_guard<std::mutex> guard(queue_mutex_);
                    queue_.push(delta);
                });
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
    server::contexts_request::done() {
        srv->contexts.context_added.remove(this);
        srv->contexts.context_removed.remove(this);
    }

    // ---------- streaming tree request -------------
    
    void
    server::tree_request::reg() {
        auto cq = srv->cq_.get();
        srv->service_.RequestStreamTree(&ctx_, &request_, &responder_, cq, cq, this);
    }

    void
    server::tree_request::called() {
        if (status == CREATED) {
            // check the request and add a listener to the corresponding tree
            // if it exists
            size_t id = request_.tree_id();
            std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
            context* ctx = srv->ids_.get_context(id);

            auto cl = ctx->lock();
            if (ctx->get_tree()) {
            }
            try {
                tree_ = srv->id_ctx_map_.at(id)->get_tree();
            } catch (const std::out_of_range& e) { 
                tree_ = nullptr;
            }
            srv->on_context_removed.add(this, [id, this] (context* ctx) {
                        size_t cid = srv->ctx_id_map_.at(ctx);
                        if (cid != id) {
                            std::lock_guard<std::mutex> guard(queue_mutex_);
                            tree_->on_descendant_added.remove(this);
                            tree_->on_descendant_removed.remove(this);
                            tree_ = nullptr;
                            alarm();
                        }
                    });
            if (tree_) {
                tree_->on_dispose.add(this, [this]() {
                            std::lock_guard<std::mutex> guard(queue_mutex_);
                            tree_->on_descendant_added.remove(this);
                            tree_->on_descendant_removed.remove(this);
                            tree_ = nullptr;
                            alarm();
                        });
                tree_->on_descendant_added.add(this, [this](node* n) {
                            std::lock_guard<std::mutex> guard(queue_mutex_);
                            alarm();
                        });
                tree_->on_descendant_removed.add(this, [this](node* n) {
                            std::lock_guard<std::mutex> guard(queue_mutex_);
                            delta.set_type(proto::DeltaType::REMOVED);
                            alarm();
                        });
            }
        }
        std::lock_guard<std::mutex> guard(queue_mutex_);
        if (tree_ == nullptr) {
            mark_finished();
            responder_.Finish(grpc::Status::OK, this);
        } else if (!queue_.empty()) {
            const proto::TreeDelta& delta = queue_.front();
            responder_.Write(delta, this);
            queue_.pop();
        }
    }

    void
    server::tree_request::done() {
        srv->on_context_removed.remove(this);
    }
}
