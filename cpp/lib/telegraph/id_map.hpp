#ifndef __TELEGRAPH_ID_MAP_HPP__
#define __TELEGRAPH_ID_MAP_HPP__

#include <unordered_map>
#include <cinttypes>

namespace telegraph {
    class node;
    class tree;

    class id_map {
    public:
        id_map();

        void add(node* n);
        void add_leaves(tree* t);

        inline int32_t operator[](const node* n) const {
            try { 
                return node_id_map_.at((node*) n);
            } catch (const std::out_of_range& e) {
                return -1;
            }
        }

        inline node* operator[](int32_t id) const {
            try { 
                return id_node_map_.at(id);
            } catch (const std::out_of_range& e) {
                return nullptr;
            }
        }
    private:
        std::unordered_map<node*, int32_t> node_id_map_;
        std::unordered_map<int32_t, node*> id_node_map_;
        int32_t next_id_;
    };
}

#endif
