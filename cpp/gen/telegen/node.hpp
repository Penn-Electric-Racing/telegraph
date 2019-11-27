#ifndef __TELEGEN_NODE_HPP__
#define __TELEGEN_NODE_HPP__

#include <stdint.h>
#include "common.nanopb.h"

namespace telegen {
    class node {
    public:
        constexpr node(int32_t id, const char* name, 
                const char* pretty, const char* desc) : 
            id_(id), name_(name), 
            pretty_(pretty), desc_(desc) {}

        constexpr int32_t get_id() const { return id_; }

        // use char* so we can store everything in data section
        constexpr const char* get_name() const { return name_; }
        constexpr const char* get_pretty() const { return pretty_; }
        constexpr const char* get_desc() const { return pretty_; }

        // encode this node into a node protobuffer descriptor
        virtual void encode(telegraph_proto_Node* n, int32_t parent=-1) const = 0;
    private:
        int32_t id_;
        const char* name_;
        const char* pretty_;
        const char* desc_;
    };
}

#endif
