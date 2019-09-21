#ifndef __TELEGRAPH_GEN_NODE_HPP__
#define __TELEGRAPH_GEN_NODE_HPP__

namespace telegraph::gen {
    class node {
    public:
        constexpr node(int32_t id) : id_(id) {}

        constexpr int32_t get_id() const { return id_; }
    private:
        int32_t id_;
    };
}

#endif
