#include "dummy_device.hpp"

namespace telegraph {
    dummy_device::dummy_device(io::io_context& ioc, const std::string_view& name, 
                                std::unique_ptr<node>&& tree)
                           : local_context(ioc, name, "dummy_device", 
                                    params{}, std::move(tree))  {}

    void
    dummy_device::add_publisher(const variable* v, const publisher_ptr& p) {
        publishers_.emplace(v, p);
    }

    void
    dummy_device::add_handler(const action* a, const dummy_device::handler& h) {
        handlers_.emplace(a, h);
    }

    params_stream_ptr 
    dummy_device::request(io::yield_ctx&, const params& p) {
        return nullptr;
    }

    subscription_ptr
    dummy_device::subscribe(io::yield_ctx&, const variable* v,
                            float min_interval, float max_interval,
                            float timeout) {
        auto it = publishers_.find(v);
        if (it == publishers_.end()) return nullptr;
        auto p = it->second;
        if (!p) return nullptr;
        return p->subscribe(min_interval, max_interval);
    }
   local_context_ptr 
    dummy_device::create(io::yield_ctx&, io::io_context& ioc,
            const std::string_view& name, const std::string_view& type,
            const params& p, sources_map&& srcs) {
        auto childA = new variable(2, "a", "A", "", value_type::Float);
        auto childB = new variable(3, "b", "B", "", value_type::Bool);

        std::vector<std::string> labels{"On", "Off"};
        auto status_type = value_type("Status", std::move(labels));
        auto childC = new variable(4, "b", "B", "", status_type);

        std::vector<node*> children{childA, childB, childC};
        auto root = std::make_unique<group>(1, "foo", "Foo", "", "", 1, std::move(children));
        auto dev = std::make_shared<dummy_device>(ioc, name, std::move(root));
        // spawn data-pushing loop....
        std::weak_ptr<dummy_device> w{dev};
        return dev;
    }
}