#ifndef __TELEGRAPH_LOCAL_DEVICE_HPP__
#define __TELEGRAPH_LOCAL_DEVICE_HPP__

#include "namespace.hpp"

#include "../common/params.hpp"
#include "../common/adapter.hpp"
#include "../common/nodes.hpp"

#include "../utils/io_fwd.hpp"

#include <string>
#include <memory>
#include <unordered_map>
#include <deque>
#include <iostream>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>

#include "stream.pb.h"

namespace telegraph {
    class device_io_worker;
    class device : public local_context {
    private:
        std::deque<stream::Packet> write_queue_;
        io::streambuf write_buf_;
        io::streambuf read_buf_;

        bool one_start_;
        bool decoding_;
        io::streambuf decode_buf_;

        uint32_t req_id_;
        struct req {
            io::deadline_timer* timer;
            stream::Packet* packet;
            constexpr req(io::deadline_timer* t, stream::Packet* p) 
                : timer(t), packet(p) {}
        };

        std::unordered_map<uint32_t, req> reqs_;

        // subscription adapters
        std::unordered_map<node::id, std::shared_ptr<adapter_base>> adapters_;

        io::serial_port port_;
    public:
        device(io::io_context& ioc, const std::string& name, const std::string& port, int baud);
        ~device();

        // init should be called right after construction! (this is done by create)
        // or the context will not have a tree (this is done by device_io_task)
        void init(io::yield_ctx&, int millisec_timeout);

        bool ping(io::yield_ctx&, bool wait=true, int millisec_timeout=50);
        node* fetch_node(io::yield_ctx&, node::id id);

        // no querying
        params_stream_ptr request(io::yield_ctx&, const params& p) { return nullptr; }

        subscription_ptr subscribe(io::yield_ctx& ctx, const variable* v,
                                float min_interval, float max_interval, 
                                float timeout) override;
        value call(io::yield_ctx& ctx, action* a, value v, float timeout);

        void destroy(io::yield_ctx& ctx) override;

        // path-based overloads
        subscription_ptr subscribe(io::yield_ctx& ctx, 
                                const std::vector<std::string_view>& path,
                                float min_interval, float max_interval, 
                                float timeout) override {
            auto n =  tree_->from_path(path);
            auto v = dynamic_cast<variable*>(n);
            if (!v) return nullptr;
            return subscribe(ctx, v, min_interval, max_interval, timeout);
        }

        value call(io::yield_ctx& ctx, 
                        const std::vector<std::string_view>& path, 
                        value v, float timeout) override {
            auto a = dynamic_cast<action*>(tree_->from_path(path));
            if (!a) return value::invalid();
            return call(ctx, a, v, timeout);
        }

        // unimplemented context functions
        bool write_data(io::yield_ctx&, variable* v, 
                const std::vector<datapoint>& d) override { return false; }

        bool write_data(io::yield_ctx&, 
                const std::vector<std::string_view>& path, 
                const std::vector<datapoint>& d) override { return false; }

        data_query_ptr query_data(io::yield_ctx& yield, 
                                            const variable * n) override { return nullptr; }
        data_query_ptr query_data(io::yield_ctx& yield, 
                                const std::vector<std::string_view>& p) override { return nullptr; }

        static local_context_ptr create(io::yield_ctx&, io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type,
                const params& p);
    private:
        std::shared_ptr<device> shared_device_this() {
            return std::static_pointer_cast<device>(shared_from_this());
        }
        std::weak_ptr<device> weak_device_this() {
            return std::weak_ptr<device>{std::static_pointer_cast<device>(shared_from_this())};
        }

        // will queue a write out
        // called from within the port executing strand
        void do_reading(size_t requested = 0); // requested of 0 just read any amount
        void on_read(const boost::system::error_code& ec, size_t transferred);

        void do_write_next();
        void write_packet(stream::Packet&& p);
        void on_read(stream::Packet&& p);
    };

    class device_scanner : public local_component {
    private:
        std::unordered_map<params_stream*, 
                std::weak_ptr<params_stream>> requests_;
        std::vector<std::string> last_devices_;
    public:
        device_scanner(io::io_context& ioc, const std::string_view& name);
        ~device_scanner();

        void init();

        params_stream_ptr request(io::yield_ctx&, const params& p) override;

        static local_component_ptr create(io::yield_ctx&, io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type,
                const params& p);
    };
}
#endif
