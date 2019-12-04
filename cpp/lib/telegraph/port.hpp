#ifndef __TELEGRAPH_PORT_HPP__
#define __TELEGRAPH_PORT_HPP__

#include <memory>
#include <iostream>
#include <vector>

#include "stream.pb.h"

namespace telegraph {
    class tree;
    class node;

    class port {
    public:
        port();
        ~port();

        void write(const proto::StreamEvent& event);
        void flush();

        void read(proto::StreamEvent* event);

        // returns
        void open(const std::shared_ptr<std::iostream>& stream);
        void close();

        std::vector<node*> fetch_children(int32_t parent_id);
        tree* fetch_tree();

        // read/write events from the port
        void run();
    private:
        std::shared_ptr<std::iostream> stream_;
    };
}

#endif
