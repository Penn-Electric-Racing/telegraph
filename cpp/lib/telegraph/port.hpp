#ifndef __TELEGRAPH_PORT_HPP__
#define __TELEGRAPH_PORT_HPP__

#include "utils/serial_port.hpp"

#include <memory>
#include <vector>

#include "stream.pb.h"

namespace telegraph {
    class tree;
    class node;

    class port {
    public:
        port(const std::string& name, int baud, long timeout=-1);
        ~port();

        // TODO: Not yet implemented
        void read(proto::StreamEvent* event);

        void write(const proto::StreamEvent& event);
        void flush();


        std::vector<node*> fetch_children(int32_t parent_id);
        tree* fetch_tree();
    private:
        serial_port port_;
    };
}

#endif
