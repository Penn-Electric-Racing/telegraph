#include "port.hpp"

#include "tree.hpp"

#include "nodes/node.hpp"
#include "nodes/group.hpp"

namespace telegraph {

    port::port() : stream_() {}

    port::~port() {
        if (stream_) close();
    }

    void
    port::write(const proto::StreamEvent& event) {
    }

    void
    port::open(const std::shared_ptr<std::iostream>& stream) {
        stream_ = stream;
    }

    std::vector<node*>
    port::fetch_children(int32_t parent_id) {
        proto::StreamEvent event;
        proto::ChildrenRequest *req = event.mutable_children_request();
        req->set_parent_id(parent_id);

        write(event);
        flush()

        proto::StreamEvent event;
        while (!event.has_children()) {
            read(&event);
        }

        proto::StreamEvent*
    }

    tree*
    port::fetch_tree() {
        std::vector<node*> roots = fetch_children(-1); // fetch roots
        if (roots.size() != 1) {
            throw missing_error("Must have exactly one root!");
        }
        node* r = roots[0];
        if (group* g = dynamic_cast<group*>(r)) {
            return new tree(g);
        }
        throw missing_error("root must be of type group!");
    }

    void
    port::close() {
        stream_ = nullptr;
    }

    void
    port::run() {
    }
}
