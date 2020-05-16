#ifndef __TELEGRAPH_CONNECTION_HPP__
#define __TELEGRAPH_CONNECTION_HPP__

#include "../utils/io.hpp"

#include "api.pb.h"

#include <unordered_map>
#include <functional>

#include <boost/asio/deadline_timer.hpp>

namespace telegraph {
    namespace api {
        class Packet;
    }

    class connection {
    private:
        using handler = std::function<void(io::yield_ctx&, const api::Packet& p)>;

        io::io_context& ioc_;
        bool count_down_;
        int32_t counter_;

        struct response {
            io::deadline_timer& timeout;
            api::Packet& packet;

            response(io::deadline_timer& timeout, api::Packet& res) : 
                timeout(timeout), packet(res) {}
        };

        std::unordered_map<int32_t, response> open_requests_;
        std::unordered_map<int32_t, handler> open_streams_;
        std::unordered_map<api::Packet::PayloadCase, handler> handlers_;
    public:
        connection(io::io_context& ioc, bool count_down);
        ~connection();

		// both send/received should be non-blocking

        // this will not return until the packet has been fully
        // processed. that way request order is preserved
        void received(io::yield_ctx& yield, const api::Packet& p);

        virtual void send(api::Packet&& p) = 0;

        // request-response pair
        api::Packet request_response(io::yield_ctx& yield, api::Packet&& req);
        api::Packet request_stream(io::yield_ctx& yield, api::Packet&& req, const handler& cb);

        void set_handler(api::Packet::PayloadCase c, const handler& h);
        void set_stream_cb(int32_t req_id, const handler& h);

        void write_back(int32_t req_id, api::Packet&& p);

        void close_stream(int32_t req_id);
    };
}

#endif
