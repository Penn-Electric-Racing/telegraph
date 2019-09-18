#include "context.hpp"

namespace per {
    context::context(const std::string& name) : name_(name), tree_(nullptr) {}

    context::~context() {
        if (tree_) tree_->on_dispose.remove(this);
        on_dispose();
        delete tree_;
    }

    context_set::context_set() : 
        context_added(), context_removed(), m_(), contexts_() {}
}
