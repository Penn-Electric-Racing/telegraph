#ifndef __TELEGRAPH_VARIABLE_HPP__
#define __TELEGRAPH_VARIABLE_HPP__

#include "node.hpp"
#include "../value.hpp"
#include "../datapoint.hpp"

#include <functional>
#include <memory>

namespace telegraph {
    class subscription {
    public:
        inline subscription(uint64_t min_interval, uint64_t max_interval) :
                                on_data(), on_cancel(), 
                                min_interval_(min_interval), max_interval_(max_interval), 
                                cancelled_(false) {}

        constexpr bool is_cancelled() const { return cancelled_; }

        constexpr uint64_t get_min_interval() const { return min_interval_; }
        constexpr uint64_t get_max_interval() const { return max_interval_; }

        inline void cancel() { on_cancel(); }

        signal<datapoint> on_data;
        signal<> on_cancel;
    private:
        uint64_t min_interval_;
        uint64_t max_interval_;
        bool cancelled_;
    };

    class variable : public node {
    public:

        variable(const std::string& name, 
                const std::string& pretty, const std::string& desc, const type t);

        inline void set_type(const type& t) { type_ = t; }
        constexpr const type& get_type() const { return type_; }

        std::shared_ptr<subscription> subscribe(uint64_t min_interval=0, uint64_t max_interval=0);

        void print(std::ostream& o, int ident=0) const override;

        signal<std::shared_ptr<subscription>&> on_subscribe;
    private:
        type type_;
    };

    using subscription_handle = std::shared_ptr<subscription>;
}

#endif
