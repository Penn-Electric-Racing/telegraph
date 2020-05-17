#ifndef __TELEGRAPH_LOCAL_DUMMY_DEVICE_HPP__
#define __TELEGRAPH_LOCAL_DUMMY_DEVICE_HPP__

#include "namespace.hpp"
#include "../common/publisher.hpp"
#include "../common/nodes.hpp"

#include <string_view>

namespace telegraph {
    class dummy_device : public local_context {
    public:
        using handler = std::function<void(io::yield_ctx&, value)>;
    private:
        std::unordered_map<const variable*, publisher_ptr> publishers_;
        std::unordered_map<const action*, handler> handlers_;
    public:
        dummy_device(io::io_context& ioc, const std::string_view& name,
                    std::unique_ptr<node>&& s);
        ~dummy_device();

        void add_publisher(const variable* v, const publisher_ptr& p);
        void add_handler(const action* a, const handler& h);

        params_stream_ptr request(io::yield_ctx&, const params& p) override;

        subscription_ptr subscribe(io::yield_ctx& ctx,
                const variable* v, 
                float min_interval, float max_interval, 
                float timeout) override;

        value call(io::yield_ctx& yield, action* a, 
                            value v, float timeout) override;


        subscription_ptr subscribe(io::yield_ctx& yield,
                const std::vector<std::string_view>& path,
                float min_interval, float max_interval, 
                float timeout) override {
            auto v = dynamic_cast<variable*>(tree_->from_path(path));
            if (!v) return nullptr;
            return subscribe(yield, v, min_interval, max_interval, timeout);
        }

        value call(io::yield_ctx& yield, 
                    const std::vector<std::string_view>& path, 
                    value v, float timeout) override {
            auto a = dynamic_cast<action*>(tree_->from_path(path));
            if (!a) return value::invalid();
            return call(yield, a, v, timeout);
        }

        bool write_data(io::yield_ctx& yield, 
                variable* v, 
                const std::vector<data_point>& data) override {
            return false;
        }
        bool write_data(io::yield_ctx& yield, 
                const std::vector<std::string_view>&, 
                const std::vector<data_point>& data) override {
            return false;
        }

        data_query_ptr query_data(io::yield_ctx& yield, 
                                    const variable* v) override {
            return nullptr;
        }
        data_query_ptr query_data(io::yield_ctx& yield, 
                const std::vector<std::string_view>& v) override {
            return nullptr;
        }

        static local_context_ptr create(io::yield_ctx&, io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type,
                const params& p, sources_map&& srcs);
    };
}

#endif