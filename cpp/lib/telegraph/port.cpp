#include "port.hpp"

#include "tree.hpp"
#include "errors.hpp"

#include "nodes/node.hpp"
#include "nodes/group.hpp"

static uint32_t fletcher32(const uint16_t *data, size_t len) {
    uint32_t c0, c1;
    unsigned int i;

    for (c0 = c1 = 0; len >= 360; len -= 360) {
        for (i = 0; i < 360; ++i) {
            c0 = c0 + *data++;
            c1 = c1 + c0;
        }
        c0 = c0 % 65535;
        c1 = c1 % 65535;
    }
    for (i = 0; i < len; ++i) {
        c0 = c0 + *data++;
        c1 = c1 + c0;
    }
    c0 = c0 % 65535;
    c1 = c1 % 65535;
    return (c1 << 16 | c0);
}

namespace telegraph {
    port::port(const std::string& name, int baud, long timeout) : port_(name, baud, timeout) {}
    port::~port() {}

    void
    port::write(const proto::StreamEvent& event) {
        std::string data;
        if (!event.SerializeToString(&data)) {
            throw io_error("failed to serailize");
        }
        // write header
        port_ << (uint8_t) (data.size() >> 2);
        size_t num_bytes = (data.size() >> 2) << 2;
        data.append(num_bytes - data.size(), (char) 0);
        port_ << data;
        // write the checksum
        fletcher32(reinterpret_cast<const uint16_t*>(data.c_str()), num_bytes >> 1);
    }

    void
    port::read(proto::StreamEvent* event) {
    }

    void
    port::flush() {
    }

    std::vector<node*>
    port::fetch_children(int32_t parent_id) {
        proto::StreamEvent event;
        event.mutable_children_request();
        write(event);
        flush();

        while (!event.has_children()) {
            read(&event);
        }

        return std::vector<node*>();
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

}
