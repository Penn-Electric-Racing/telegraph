#ifndef __TELEGEN_INTERFACE_HPP__
#define __TELEGEN_INTERFACE_HPP__

#include "promise.hpp"
#include "value.hpp"

#include <memory>
#include <cstdint>

namespace telegen {
    class variable_base;
    class action_base;
    class subscription;
    using subscription_ptr = std::unique_ptr<subscription>;

    class interface {
    public:
        virtual promise<subscription_ptr> subscribe(variable_base* v,  
                        int32_t min_interval, int32_t max_interval) = 0;
        virtual promise<value> call(action_base* a, const value& arg) = 0;
    };

    // TODO: The local interface should
    // have some sort of a virtual get_next_time() and continue() so
    // that all the publishers can be combined under
    // a single coroutine so that we don't have to poll
    // every single publisher to see if it is ready to publish
    class local_interface : public interface {
    };

    template<typename T, typename Clock>
        class publisher : public interface {
        public:
            publisher(Clock& c) : clock_(c) {}

            promise<subscription_ptr> subscribe(variable_base* v,
                        int32_t min_inteval, int32_t max_interval) {
                return promise<subscription_ptr>();
            }
        private:
            Clock& clock_;
        };

    template<typename Arg, typename Ret, typename Clock>
        class action_handler {
        public:
        };
}

#endif
