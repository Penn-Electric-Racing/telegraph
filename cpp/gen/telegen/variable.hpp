#ifndef __TELEGEN_VARIABLE_HPP__
#define __TELEGEN_VARIABLE_HPP__

#include "node.hpp"
#include "interface.hpp"
#include "storage_traits.hpp"

#include <vector>
#include <functional>

namespace telegen {
    class generic_subscription {
    public:
        constexpr generic_subscription(int32_t min_interval, int32_t max_interval,
                                        interface* origin) : 
                                        min_interval_(min_interval),
                                        max_interval_(max_interval),
                                        origin_(origin) {}
    private:
        int32_t min_interval_;
        int32_t max_interval_;

        /// interface from which this subscription request originated,
        /// null means it is local (i.e from the device)
        interface* origin_; 
    };

    template<typename T>
        class subscription : public generic_subscription {
        public:
            subscription(int32_t min_interval, int32_t max_interval, 
                            interface* origin) :
                                generic_subscription(min_interval, max_interval, origin), 
                                handlers_() {}

            void add(std::function<void(const T&)>&& f) {
                handlers_.push_back(std::move(f));
            }

            void notify(const T& val) {
                for (auto& f : handlers_) f(val);
            }
        private:
            std::vector<std::function<void(const T&)>> handlers_;
        };

    class generic_variable : public node {
    public:
        constexpr generic_variable(int32_t id, const char* name, const char* pretty,
                                    const char* desc) : node(id, name, pretty, desc) {}
    };

    template<typename T>
        class variable : public generic_variable {
        public:
            using store_type = typename storage_traits<T>::type;

            constexpr variable(int32_t id, const char* name, const char* pretty,
                                const char* desc) : generic_variable(id, name, pretty, desc) {}

            // write to this variable
            variable& operator<<(const T& t) {
                last_ = t;
            }

            void add_interface(interface* i) {
                interfaces_.push_back(i);
            }

            subscription<T>* subscribe(int32_t min_interval, int32_t max_interval, 
                                        interface* interface=nullptr) {
                subscriptions_.push_back(subscription<T>(min_interval, max_interval, interface));
                subscription<T>* s = &subscriptions_.back();

                for (auto * i : interfaces_) {
                    i->subscribed(this, s);
                }
                return s;
            }

            void pack(telegraph_proto_Variable* v, int32_t parent) const {
                v->id = id_;
                v->parent = parent;

                // pass a callback to encode the name
                v->name.funcs.encode = [](pb_ostream_t* stream, 
                        const pb_field_t* file,void* const* arg) {
                    variable<T>* v = reinterpret_cast<variable<T>*>(*arg);
                };
                v->name.arg = this;
            }

            void pack(telegraph_proto_Node* n, int32_t parent) const override {
            }
        protected:
            store_type last_;
            std::vector<interface*> interfaces_; // interfaces listen for subscription changes
            std::vector<subscription<T>> subscriptions_; // subscriptions 
                                                       // (in a list so we can return pointers)
        };
}

#endif
