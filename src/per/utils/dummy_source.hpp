#ifndef __PER_UTILS_DUMMY_SOURCE_HPP__
#define __PER_UTILS_DUMMY_SOURCE_HPP__

#include "../nodes/variable.hpp"

#include <unordered_set>
#include <unordered_map>

namespace per {
    class dummy_source : public variable::source {
    public:
        class subscription : public variable::subscription {
            friend class dummy_source;
        public:
            inline subscription(uint64_t interval, variable* v, 
                                        dummy_source* src) : 
                        interval_(interval), src_(src), var_(v), funcs_() {}

            void send(const value& v);

            void notify(const std::function<void(value)>& func) override;
            void unsubscribe() override;
        private:
            uint64_t interval_;
            dummy_source* src_;
            variable* var_;
            std::vector<std::function<void(value)>> funcs_;
        };

        dummy_source();

        void update(variable* var, const value& v);

        void unsubscribe(subscription* s);

        variable::subscription* subscribe(variable* v, uint64_t interval) override;
    private:
        std::unordered_set<subscription*> subscribers_;
        std::unordered_map<variable*, std::unordered_set<subscription*>> vars_;
    };
}

#endif
