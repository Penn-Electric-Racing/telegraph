#ifndef __TELEGRAPH_LOCAL_CONTAINER_HPP__
#define __TELEGRAPH_LOCAL_CONTAINER_HPP__

#include "../utils/io_fwd.hpp"
#include "../common/namespace.hpp"
#include "../common/nodes.hpp"
#include "../common/data.hpp"

#include "namespace.hpp"

#include <string>
#include <string_view>
#include <memory>
#include <unordered_set>

namespace telegraph {
    class container : public local_context {
    private:
        std::vector<context_ptr> mounts_;
        std::shared_ptr<node> tree_;
    public:
        container(io::io_context& ioc, const std::string_view& name, 
                    std::unique_ptr<node>&& tree);

        params_stream_ptr request(io::yield_ctx&, const params& p) override;

        // forwarding functions
        subscription_ptr subscribe(io::yield_ctx& ctx,
                const std::vector<std::string_view>& variable,
                float min_interval, float max_interval, float timeout) override;

        subscription_ptr subscribe(io::yield_ctx& ctx,
                const variable* v, 
                float min_interval, float max_interval, float timeout) override;

        value call(io::yield_ctx& yield, action* a, value v, float timeout) override {
            return value::invalid();
        }
        value call(io::yield_ctx& yield, 
            const std::vector<std::string_view>& a, value v, float timeout) override {
            return value::invalid();
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

        void mount(io::yield_ctx& y, const context_ptr& src) override { 
            local_context::mount(y, src);
            mounts_.push_back(src); 
        }
        void unmount(io::yield_ctx& y, const context_ptr& src) override {
            local_context::unmount(y, src);
            mounts_.erase(std::remove(mounts_.begin(), 
                        mounts_.end(), src), mounts_.end());
        }
        std::shared_ptr<node> fetch(io::yield_ctx&) override { return tree_; }

        static local_context_ptr create(io::yield_ctx&, io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type,
                const params& p, sources_map&& srcs);
    };
}

#endif