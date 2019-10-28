#include "server.hpp"

#include "context.hpp"

#include <iostream>

#include "nodes/group.hpp"
#include "nodes/action.hpp"
#include "nodes/variable.hpp"
#include "nodes/stream.hpp"

namespace telegraph {

    id_manager::id_manager() : ctx_id_counter_(0),
                               id_ctx_map_(), ctx_id_map_(),
                               node_id_counter_(0), 
                               id_node_map_(), node_id_map_(),
                               ctx_node_ids_() {}

    void
    id_manager::add_context(context* ctx) {
        if (ctx_id_map_.find(ctx) != ctx_id_map_.end()) return;
        int32_t id = ctx_id_counter_++;
        ctx_id_map_[ctx] = id;
        id_ctx_map_[id] = ctx;
    }
    void
    id_manager::remove_context(context* ctx) {
        try {
            int32_t cid = ctx_id_map_.at(ctx);
            id_ctx_map_.erase(cid);
            ctx_id_map_.erase(ctx);
        } catch (std::out_of_range& e) {
            return;
        }
        try {
            std::unordered_set<int32_t>& v = ctx_node_ids_.at(ctx);
            for (int32_t i : v) {
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
        int32_t id = node_id_counter_++;
        node_id_map_[n] = id;
        id_node_map_[id] = n;
        ctx_node_ids_[ctx].insert(id);
    }
    void
    id_manager::remove_node(context* ctx, node* n) {
        try { 
            int32_t id = node_id_map_.at(n);
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
        open_requests.insert(new tree_request(this));
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
        std::lock_guard<std::mutex> guard(queue_mutex_);
        if (status == CREATED) { // first time called! queue existing contexts
                                 // and adds listeners
            auto& contexts = srv->contexts;

            // lock the contexts
            {
                std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                auto cl = contexts.lock();

                for (context* ctx : contexts.set()) {
                    auto l = ctx->lock();
                    int32_t id = srv->ids_.get_id(ctx);
                    proto::ContextDelta delta;
                    delta.set_type(proto::DeltaType::INITIAL);
                    delta.mutable_context()->set_id(id);
                    delta.mutable_context()->set_name(ctx->get_name());
                    queue_.push(delta);
                }

                proto::ContextDelta delta;
                delta.set_type(proto::DeltaType::INITIALIZED);
                queue_.push(delta);
            }

            auto cl = contexts.lock();
            contexts.context_added.add(this, [this](context* ctx) {
                    proto::ContextDelta delta;
                    delta.set_type(proto::DeltaType::ADDED);
                    {
                        // lock the ids and the context
                        std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                        auto l = ctx->lock();
                        int32_t id = srv->ids_.get_id(ctx);
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
                        int32_t id = srv->ids_.get_id(ctx);
                        delta.mutable_context()->set_id(id);
                        delta.mutable_context()->set_name(ctx->get_name());
                    }
                    std::lock_guard<std::mutex> guard(queue_mutex_);
                    queue_.push(delta);
                });
        }
        // write a single context
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

    // ---------- tree request -------------

    static void pack_type(proto::Type* pt, const type& t) {
        for (auto label : t.get_labels()) {
            pt->add_labels(label);
        }
        pt->set_type(t.to_proto_buffer_class());
    }

    // helper for serializing the node into a added tree delta
    static void pack_node(proto::TreeDelta* delta, int32_t parent_id, int32_t id, const node* n) {
        if (dynamic_cast<const group*>(n) != nullptr) {
            const group* g = dynamic_cast<const group*>(n);
            proto::Group* dg = delta->mutable_group();
            dg->set_id(id);
            dg->set_parent(parent_id);
            dg->set_name(g->get_name());
            dg->set_pretty(g->get_pretty());
            dg->set_desc(g->get_desc());

            dg->set_schema(g->get_schema());
            dg->set_version(g->get_version());
        } else if (dynamic_cast<const action*>(n) != nullptr) {
            const action* a = dynamic_cast<const action*>(n);
            proto::Action* da = delta->mutable_action();
            da->set_id(id);
            da->set_parent(parent_id);
            da->set_name(a->get_name());
            da->set_pretty(a->get_pretty());
            da->set_desc(a->get_desc());

            pack_type(da->mutable_arg_type(), a->get_arg_type());
            pack_type(da->mutable_ret_type(), a->get_ret_type());
        } else if (dynamic_cast<const variable*>(n) != nullptr) {
            const variable* v = dynamic_cast<const variable*>(n);
            proto::Variable* dv = delta->mutable_var();
            dv->set_id(id);
            dv->set_parent(parent_id);
            dv->set_name(v->get_name());
            dv->set_pretty(v->get_pretty());
            dv->set_desc(v->get_desc());

            pack_type(dv->mutable_data_type(), v->get_type());
        } else if (dynamic_cast<const stream*>(n) != nullptr) {
            //const stream* s = dynamic_cast<const stream*>(n);
            std::cerr << "cannot pack stream node!" << std::endl;
        }
    }

    void
    server::tree_request::reg() {
        auto cq = srv->cq_.get();
        srv->service_.RequestStreamTree(&ctx_, &request_, &responder_, cq, cq, this);
    }


    void
    server::tree_request::called() {
        std::lock_guard<std::mutex> guard(req_mutex_);
        if (status == CREATED) {
            // check the request and add a listener to the corresponding tree
            // if it exists
            int32_t id = request_.tree_id();
            context* ctx = nullptr;

            {
                std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                ctx = srv->ids_.get_context(id);
            }

            // if we could not find the context
            // raise an error
            if (!ctx) {
                std::cerr << "tree_request: ctx " << id << ": not found" << std::endl;

                mark_finished();
                responder_.Finish(grpc::Status::CANCELLED, this);
                return;
            }

            auto ctx_lock = ctx->lock();

            // get a context tree handle
            // for this particular context
            tree_ = ctx->get();

            // now list all of the nodes
            // since we have locked the context nobody else
            // should be messing with the tree at this point
            std::vector<node*> nodes = tree_->nodes();
            for (node* n : nodes) {
                int32_t node_id = 0;
                int32_t parent_id = 0;
                {
                    std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                    node_id = srv->ids_.get_id(n);
                    parent_id = srv->ids_.get_id(n->get_parent());
                }
                proto::TreeDelta delta;
                delta.set_type(proto::DeltaType::INITIAL);
                pack_node(&delta, node_id, parent_id, n);
                queue_.push(delta);
            }

            // now put in the queue that we have initialized the tree
            proto::TreeDelta finished;
            finished.set_type(proto::DeltaType::INITIALIZED);
            queue_.push(finished);

            // if this particular context gets removed from
            // the server make the request as finished
            {
                auto acl = srv->contexts.lock();
                srv->contexts.context_removed.add(this, [id, this] (context* ctx) {
                    int32_t ctx_id = 0;
                    {
                        std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                        ctx_id = srv->ids_.get_id(ctx);
                    }
                    if (ctx_id != id) {
                        {
                            std::lock_guard<std::mutex> guard(req_mutex_);
                            mark_finished();
                        }
                        alarm();
                    }
                });
            }
            tree_->on_descendant_added.add(this, [this](node* n) {
                int32_t node_id = 0;
                int32_t parent_id = 0;
                {
                    std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                    node_id = srv->ids_.get_id(n);
                    parent_id = srv->ids_.get_id(n->get_parent());
                }
                proto::TreeDelta delta;
                delta.set_type(proto::DeltaType::ADDED);
                {
                    auto ctx_lock = tree_.ctx()->lock();
                    pack_node(&delta, node_id, parent_id, n);
                }
                {
                    std::lock_guard<std::mutex> guard(req_mutex_);
                    queue_.push(delta);
                }
                alarm();
            });
            tree_->on_descendant_removed.add(this, [this](node* n) {
                int32_t node_id = 0;
                {
                    std::lock_guard<std::recursive_mutex> il(srv->ids_mutex_);
                    node_id = srv->ids_.get_id(n);
                }
                proto::TreeDelta delta;
                delta.set_type(proto::DeltaType::REMOVED);
                delta.set_id(node_id);
                {
                    std::lock_guard<std::mutex> guard(req_mutex_);
                    queue_.push(delta);
                }
                alarm();
            });
        }
        if (!queue_.empty()) {
            const proto::TreeDelta& delta = queue_.front();
            responder_.Write(delta, this);
            queue_.pop();
        }
    }

    void
    server::tree_request::done() {
        auto ctxs_lock = srv->contexts.lock();
        srv->contexts.context_removed.remove(this);

        if (tree_) {
            auto ctx_lock = tree_.ctx()->lock();
            tree_->on_descendant_added.remove(this);
            tree_->on_descendant_added.remove(this);
            responder_.Finish(grpc::Status::OK, this);
        }
    }
}
