#ifndef __TELEGEN_GROUP_HPP__
#define __TELEGEN_GROUP_HPP__

#include "node.hpp"

#include "pb_encode.h"

#include <initializer_list>
#include <array>

namespace telegen {
    // TODO: Make have a maximum children size
    // rather than be template based?
    //
    // Having it be template based might increase
    // the size of the executable unreasonably
    template<size_t num_children>
        class group : public node {
        public:
            constexpr group(int32_t id, const char* name, const char* pretty, const char* desc,
                            const std::array<node*, num_children>& children) : 
                                node(id, name, pretty, desc), children_(children) {}

            constexpr const char* get_schema() const { return ""; }
            constexpr int32_t get_version() const { return 0; }

            void pack_def(telegraph_proto_Group* g, int32_t parent) const {
                g->id = get_id();
                g->parent = parent;

                g->version = get_version();

                auto string_encoder = [](pb_ostream_t* stream, const pb_field_iter_t* field, 
                        void* const* arg) {
                    const char* str = (const char*) *arg;
                    if (!pb_encode_tag_for_field(stream, field)) 
                        return false;
                    return pb_encode_string(stream, (uint8_t*) str, strlen(str));
                };

                g->name.arg = (void*) get_name();
                g->name.funcs.encode = string_encoder;

                g->pretty.arg = (void*) get_pretty();
                g->pretty.funcs.encode = string_encoder;

                g->desc.arg = (void*) get_desc();
                g->desc.funcs.encode = string_encoder;

                g->schema.arg = (void*) get_schema();
                g->schema.funcs.encode = string_encoder;
            }

            void pack_def(telegraph_proto_Node* n, int32_t parent) const override {
                n->which_node = telegraph_proto_Node_group_tag;
                pack_def(&n->node.group, parent);
            }

            bool pack_children(pb_ostream_t* os, const pb_field_iter_t* field) const override {
                for (node* n : children_) {
                    telegraph_proto_Node node;
                    n->pack_def(&node, get_id());

                    if (!pb_encode_tag_for_field(os, field)) 
                        return false;
                    if (!pb_encode_submessage(os, telegraph_proto_Node_fields, &node))
                        return false;
                }
                return true;
            }
        private:
            std::array<node*, num_children> children_;
        };
}

#endif
