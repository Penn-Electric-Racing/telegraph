#ifndef __TELEGRAPH_CONTEXT_HPP__
#define __TELEGRAPH_CONTEXT_HPP__

#include <string>
#include <mutex>
#include <unordered_set>

#include "utils/signal.hpp"
#include "tree.hpp"

namespace telegraph {
    class tree;

    /**
     * Contexts are meant to be locked explicitly by the user
     * whenever some operation is being done on the tree
     * such as pumping data to subscriptions, adding/removing nodes,
     * etc., etc.
     */
    class context {
    public:
        class handle {
        public:
            constexpr handle() : ctx_(nullptr) {}
            inline handle(context* ctx): ctx_(ctx) {
                if (ctx_) ctx_->handle_created();
            };
            inline handle(const handle& h) : ctx_(h.ctx_) {
                if (ctx_) ctx_->handle_created();
            }
            inline handle(handle&& h) : ctx_(h.ctx_) { h.ctx_ = nullptr; }

            inline ~handle() { 
                if (ctx_) ctx_->handle_destroyed();
            }

            constexpr context* ctx() { return ctx_; }

            inline handle& operator=(handle&& h) {
                if (ctx_) ctx_->handle_destroyed();
                ctx_ = h.ctx_;
                return *this;
            }

            inline handle& operator=(const handle& h) {
                if (ctx_) ctx_->handle_destroyed();
                ctx_ = h.ctx_;
                if (ctx_) ctx_->handle_created();
                return *this;
            }
            constexpr tree& operator*() {
                return *ctx_->get_tree();
            }
            constexpr const tree& operator*() const {
                return *ctx_->get_tree();
            }
            constexpr tree* operator->() {
                return ctx_->get_tree();
            }
            constexpr const tree* operator->() const {
                return ctx_->get_tree();
            }

            constexpr operator bool() const {
                return ctx_ != nullptr && ctx_->get_tree() != nullptr;
            }
        private:
            context* ctx_;
        };
        friend class handle;

        using lock_guard = std::lock_guard<std::recursive_mutex>;

        context(const std::string& name);
        ~context();

        inline lock_guard lock() const { return lock_guard(m_); }

        constexpr const std::string& get_name() const { return name_; }

        // You should not be using this normally!
        // The proper way to get access to a tree is with get()
        // which gets a handle to the tree
        constexpr const tree* get_tree() const { return tree_; }
        constexpr tree* get_tree() { return tree_; }

        inline void set_tree(tree* tree) { 
            if (tree_) tree_->on_dispose.remove(this);
            on_tree_change(tree);
            tree_ = tree; 
            tree_->on_dispose.add(this, [this] () { set_tree(nullptr); });
        }

        inline handle get() {
            return handle(this);
        }

        bool has_handles() {
            return handles_;
        }

        signal<> on_dispose;

        signal<tree*> on_tree_change;

        signal<> on_request;
        signal<> on_release;
    private:
        inline void handle_created() {
            if (handles_ == 0) {
                handles_++;
                on_request(); 
            } else {
                handles_++;
            }
        }
        inline void handle_destroyed() {
            if (handles_ == 0) {
                handles_--;
                on_release();
            } else {
                handles_--;
            }
        }

        mutable std::recursive_mutex m_;
        std::string name_;
        size_t handles_; // number of handles
        tree* tree_;
    };

    // context sets DO NOT
    // own the memory of the contexts they contain
    class context_set {
    public:
        using lock_guard = std::lock_guard<std::recursive_mutex>;

        context_set();
        ~context_set();

        inline lock_guard lock() const { return lock_guard(m_); }

        constexpr const std::unordered_set<context*>& set() { return contexts_; }

        inline void add(context* c) {
            if (!c) return;
            lock();
            contexts_.insert(c);
            context_added(c);
        }

        inline void remove(context* c) {
            if (!c) return;
            lock();
            std::unordered_set<context*>::iterator pos = 
                std::find(contexts_.begin(), contexts_.end(), c);
            if (pos != contexts_.end()) {
                contexts_.erase(pos);
                context_removed(c);
            }
        }

        signal<context*> context_added;
        signal<context*> context_removed;

        signal<> on_dispose;
    private: 
        mutable std::recursive_mutex m_;
        std::unordered_set<context*> contexts_;
    };
}

#endif
