#include "context.hpp"

#include "nodes/tree.hpp"

namespace per {
    context::context(const std::string& name) : name_(name), tree_(nullptr) {}

    context::~context() {
        on_dispose();
        delete tree_;
    }
}
