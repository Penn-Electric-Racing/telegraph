#ifndef __TELEGEN_NODES_HPP__
#define __TELEGEN_NODES_HPP__

#include "util.hpp"
#include "source.hpp"
#include "types.hpp"

#include "common.nanopb.h"
#include "pb_encode.h"

#include <cstdint>
#include <memory>
#include <functional>
#include <array>


namespace telegen {
    template<size_t N>
        using id_array = std::array<int32_t, N>;

    class node {
    public:
        constexpr node(int32_t id, const char* name, 
                const char* pretty, const char* desc) : 
            owner_(nullptr), id_(id), name_(name), 
            pretty_(pretty), desc_(desc) {}

        constexpr int32_t get_id() const { return id_; }

        // use char* so we can store everything in data section
        constexpr const char* get_name() const { return name_; }
        constexpr const char* get_pretty() const { return pretty_; }
        constexpr const char* get_desc() const { return pretty_; }

        // encode this node into a node protobuffer descriptor
        virtual void pack(telegraph_Node* n) const = 0;

        constexpr void set_owner(source* i) {
            if (!owner_) owner_ = i;
        }
    protected:
        source* owner_;
        const int32_t id_;
        const char* const name_;
        const char* const pretty_;
        const char* const desc_;
    };

    class group : public node {
    public:
        constexpr group(int32_t id, const char* name, 
                const char* pretty, const char* desc,
                const char* schema, int32_t version,
                node* const* children, size_t num_children) :
            node(id, name, pretty, desc), 
            schema_(schema), version_(version),
            children_(children), num_children_(num_children) {}

        constexpr const char* get_schema() const { return schema_; }
        constexpr int32_t get_version() const { return version_; }

        constexpr size_t num_children() const { return num_children_; }

        inline void pack(telegraph_Group* g) const {
            g->id = get_id();

            g->name.arg = (void*) get_name();
            g->pretty.arg = (void*) get_pretty();
            g->desc.arg = (void*) get_desc();

            g->schema.arg = (void*) get_schema();
            g->version = get_version();

            g->name.funcs.encode = util::proto_string_encoder;
            g->pretty.funcs.encode = util::proto_string_encoder;
            g->desc.funcs.encode = util::proto_string_encoder;
            g->schema.funcs.encode = util::proto_string_encoder;

            g->children.arg = (void*) this;
            g->children.funcs.encode = 
                [](pb_ostream_t* stream, const pb_field_iter_t* field, 
                        void* const* arg) {
                    const group* g = (const group*) *arg;
                    telegraph_Node n = telegraph_Node_init_default;
                    for (size_t i = 0; i < g->num_children(); i++) {
                        if (!pb_encode_tag_for_field(stream, field))
                            return false;
                        g->children_[i]->pack(&n);
                        if (!pb_encode_submessage(stream, 
                                    telegraph_Node_fields, &n))
                            return false;
                    }
                    return true;
                };
        }

        inline void pack(telegraph_Node* n) const override {
            n->which_node = telegraph_Node_group_tag;
            pack(&n->node.group);
        }
    private:
        const char* const schema_;
        const int32_t version_;
        node* const* const children_;
        const size_t num_children_;
    };

    class action_base : public node {
    public:
        constexpr action_base(int32_t id, const char* name, const char* pretty,
                              const char* desc) 
            : node(id, name, pretty, desc) {}
    };

    template<typename Arg, typename Ret>
        class action : public action_base {
        public:
            constexpr action(int32_t id, const char* name, 
                            const char* pretty, const char* desc,
                            const type_info<Arg>* arg_type,
                            const type_info<Ret>* ret_type) : 
                        action_base(id, name, pretty, desc),
                        arg_type_(arg_type),
                        ret_type_(ret_type) {}

            Ret call(const Arg& a) {}

            void pack(telegraph_Action* a) const {
                a->id = get_id();
                a->name.arg = (void*) get_name();
                a->pretty.arg = (void*) get_pretty();
                a->desc.arg = (void*) get_desc();
                arg_type_->pack(&a->arg_type);
                ret_type_->pack(&a->ret_type);

                a->name.funcs.encode = util::proto_string_encoder;
                a->pretty.funcs.encode = util::proto_string_encoder;
                a->desc.funcs.encode = util::proto_string_encoder;
            }

            void pack(telegraph_Node* n) const override {
                n->which_node = telegraph_Node_action_tag;
                pack(&n->node.action);
            }
        protected:
            const type_info<Arg>* const arg_type_;
            const type_info<Ret>* const ret_type_;
        };

    class subscription {
    public:
        inline subscription(uint32_t min_time, uint32_t max_time) : 
                min_interval_(min_time), max_interval_(max_time), 
                cb_(), cancel_cb_() {}
        // subscription is cancelled on destructor
        virtual inline ~subscription() {}

        virtual bool is_cancelled() = 0;
        virtual promise<> cancel() = 0;
        virtual promise<> change(uint32_t min_time, uint32_t max_time) = 0;

        constexpr uint32_t get_min_interval() const { return min_interval_; }
        constexpr uint32_t get_max_interval() const { return max_interval_; }

        // set the handler
        void handler(const std::function<void(const value&)>& cb) {
            cb_ = cb;
        }
        void cancel_handler(const std::function<void()>& cb) {
            cancel_cb_ = cb;
        }
    protected:
        uint32_t min_interval_;
        uint32_t max_interval_;
        std::function<void(const value&)> cb_;
        // note: may be invoked even after 
        // subscription object has been deleted
        std::function<void()> cancel_cb_;
    };

    using subscription_ptr = std::unique_ptr<subscription>;

    // typed subscription, movable
    template<typename T>
        class sub {
        private:
            std::unique_ptr<subscription> sub_;
            std::function<void(const T&)> tcb_;
        public:
            sub(std::unique_ptr<subscription>&& s) : sub_(std::move(s)) {}

            void handler(const std::function<void(const T&)>& cb) {
                tcb_ = cb;
                sub_->handler([this](const void* val) { tcb_((const T&) *val); });
            }
        };

    class variable_base : public node {
    public:
        constexpr variable_base(int32_t id, const char* name, const char* pretty,
                                const char* desc) : node(id, name, pretty, desc) {}
                                


        inline promise<subscription_ptr> subscribe(int32_t min_interval, int32_t max_interval) {
            if (!owner_) return promise<subscription_ptr>(promise_status::Rejected);
            return owner_->subscribe(this, min_interval, max_interval);
        }
    };

    // extending subscription classes cannot add
    // new methods/member variables since otherwise they would
    // be slicing the subscription object

    template<typename T>
        class variable : public variable_base {
        public:
            using value_type = typename type_info<T>::value_type;

            constexpr variable(int32_t id, const char* name, const char* pretty,
                                const char* desc, const type_info<T>* type) 
                :  variable_base(id, name, pretty, desc),
                   type_(type) {}

            promise<sub<T>> subs(int32_t min_interval, int32_t max_interval) {
                auto p = subscribe(min_interval, max_interval);
                // NOTE: chain involves a malloc, find an alternative?
                return p.template chain<sub<T>>([] (subscription_ptr&& s) -> sub<T> { 
                        return sub<T>(std::move(s)); 
                });
            }

            void pack(telegraph_Variable* v) const {
                v->id = get_id();
                v->name.arg = (void*) get_name();
                v->pretty.arg = (void*) get_pretty();
                v->desc.arg = (void*) get_desc();
                type_->pack(&v->data_type);

                v->name.funcs.encode = util::proto_string_encoder;
                v->pretty.funcs.encode = util::proto_string_encoder;
                v->desc.funcs.encode = util::proto_string_encoder;
            }

            void pack(telegraph_Node* n) const override {
                n->which_node = telegraph_Node_var_tag;
                pack(&n->node.var);
            }
        protected:
            const type_info<T>* const type_;
        };
}

#endif
