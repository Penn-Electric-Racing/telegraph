#ifndef __TELEGEN_ACTION_HPP__
#define __TELEGEN_ACTION_HPP__

#include "node.hpp"

namespace telegen {

    class generic_action : public node {
    public:
        constexpr generic_action(int32_t id, const char* name, const char* pretty,
                                             const char* desc) : node(id, name, pretty, desc) {}
    };

    template<typename Arg, typename Ret>
        class action : public generic_action {
        public:
            constexpr action(int32_t id, const char* name, 
                            const char* pretty, const char* desc) : 
                                generic_action(id, name, pretty, desc) {}

            Ret call(const Arg& a) {
                // do nothing for now
            }

            void pack_def(telegraph_proto_Action* v, int32_t parent) const {
            }

            void pack_def(telegraph_proto_Node* n, int32_t parent) const override {
            }
        };

    template<typename Ret>
        class action<void, Ret> : public generic_action {
        public:
            constexpr action(int32_t id, const char* name, const char* pretty,
                                         const char* desc) : generic_action(id, name, pretty, desc) {}

            Ret call() {
                // do nothing for now
            }

            void pack_def(telegraph_proto_Action* v, int32_t parent) const {
            }

            void pack_def(telegraph_proto_Node* n, int32_t parent) const override {
            }
        };
}

#endif
