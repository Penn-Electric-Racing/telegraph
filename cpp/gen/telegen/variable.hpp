#ifndef __TELEGEN_VARIABLE_HPP__
#define __TELEGEN_VARIABLE_HPP__

#include "node.hpp"
#include "interface.hpp"
#include "type_info.hpp"

#include <list>
#include <vector>
#include <functional>

namespace telegen {
    class generic_subscription {
    public:
        inline generic_subscription(int32_t min_time, int32_t max_time) : 
            object_(nullptr), cb_(), min_interval_(min_time), max_interval_(max_time) {}

        constexpr int32_t get_min_interval() const { return min_interval_; }
        constexpr int32_t get_max_interval() const { return max_interval_; }

        virtual void cancel() {}

        inline void add(const std::function<void(const void*)>& cb) {
            cb_.push_back(cb);
        }
    protected:
        inline void update(const void* v) {
            for (auto& cb : cb_) {
                cb(v);
            }
        }

        void* object_;
        int32_t min_interval_;
        int32_t max_interval_;
    private:
        std::vector<std::function<void(const void*)>> cb_;
    };

    // this creates a typed view around
    // a generic subscription
    template<typename T>
        struct subscription {
        public:
            subscription(generic_subscription&& g) : generic(std::move(g)) {}

            void add(const std::function<void(const T&)> cb) {
                // TODO: This involves a heap allocation!
                // make a separate vector containing the casted type functions
                generic.add([cb] (const void*v) { cb((const T&) *v); });
            }

            generic_subscription generic;
        };

    class generic_variable : public node {
    public:
        constexpr generic_variable(int32_t id, const char* name, const char* pretty,
                                    const char* desc) : node(id, name, pretty, desc), 
                                    owner_(nullptr) {}

        inline void set_owner(interface* i) {
            owner_ = i;
        }

        inline generic_subscription subscribe_generic(int32_t min_interval, int32_t max_interval) {
            if (!owner_) return generic_subscription(min_interval, max_interval);
            return owner_->subscribe(this, min_interval, max_interval);
        }

        virtual void copy(void* src, int8_t* tgt, int32_t bit_offset) = 0;
    private:
        interface* owner_;
    };

    // extending subscription classes cannot add
    // new methods/member variables since otherwise they would
    // be slicing the subscription object

    template<typename T>
        class variable : public generic_variable {
        public:
            using value_type = typename type_info<T>::value_type;

            constexpr variable(int32_t id, const char* name, const char* pretty,
                                const char* desc, const type_info<T>* type) : 
                generic_variable(id, name, pretty, desc),
                type_(type) {}

            subscription<T> subscribe(int32_t min_interval, int32_t max_interval) {
                return subscription<T>(subscribe_generic(min_interval, max_interval));
            }

            void copy(void* src, int8_t* tgt, int32_t bit_offset) override {
                *((T*) (tgt + (bit_offset/8)) ) =  *((T*) src);
            }

            void pack_def(telegraph_proto_Variable* v, int32_t parent) const {
                v->id = get_id();
                v->parent = parent;

                auto string_encoder = [](pb_ostream_t* stream, const pb_field_iter_t* field, 
                        void* const* arg) {
                    const char* str = (const char*) *arg;
                    if (!pb_encode_tag_for_field(stream, field)) 
                        return false;
                    return pb_encode_string(stream, (uint8_t*) str, strlen(str));
                };

                // pass a callback to encode the name
                v->name.arg = (void*) get_name();
                v->name.funcs.encode = string_encoder;

                v->pretty.arg = (void*) get_pretty();
                v->pretty.funcs.encode = string_encoder;

                // type information
            }

            void pack_def(telegraph_proto_Node* n, int32_t parent) const override {
                n->which_node = telegraph_proto_Node_var_tag;
                pack_def(&n->node.var, parent);
            }
        protected:
            const type_info<T>* type_;

            interface* owner_; 
        };
}

#endif
