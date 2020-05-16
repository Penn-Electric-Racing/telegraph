#include "connection.hpp"

#include "../utils/errors.hpp"

#include <chrono>

namespace telegraph {
    connection::connection(io::io_context& ioc, bool count_down) : 
        ioc_(ioc),
        count_down_(count_down),
        counter_(0),
        open_requests_() {}

    void
    connection::received(io::yield_ctx& yield, const api::Packet& p) {
        if (open_requests_.find(p.req_id()) != open_requests_.end()) {
            response& r = open_requests_.at(p.req_id());
            r.packet = p;
            r.timeout.cancel();
            open_requests_.erase(p.req_id());

            // don't trigger stream handler 
            // for the first reply
            return;
        }
        if (open_streams_.find(p.req_id()) != open_streams_.end()) {
            // call the stream handler
            open_streams_.at(p.req_id())(yield, p);
        }
        if (handlers_.find(p.payload_case()) != handlers_.end()) {
            handlers_.at(p.payload_case())(yield, p);
        }
    }

    api::Packet
    connection::request_response(io::yield_ctx& yield, api::Packet&& req) {
        int32_t id = counter_++;
        req.set_req_id(id);

        io::deadline_timer timeout(ioc_, boost::posix_time::seconds(1));
        api::Packet res;

        response res_handler(timeout, res);
        open_requests_.emplace(std::make_pair(id, res_handler));

        send(std::move(req));

        boost::system::error_code error;
        timeout.async_wait(yield.ctx[error]);
        if (error != io::error::operation_aborted) {
            throw io_error("request timed out");
        }

        return res;
    }

    api::Packet
    connection::request_stream(io::yield_ctx& yield, api::Packet&& req, const handler& h) {
        int32_t id = counter_++;
        req.set_req_id(id);

        io::deadline_timer timeout(ioc_, boost::posix_time::seconds(1));
        api::Packet res;

        response res_handler(timeout, res);
        open_requests_.emplace(std::make_pair(id, res_handler));
        open_streams_.emplace(std::make_pair(id, h));

        send(std::move(req));

        boost::system::error_code error;
        timeout.async_wait(yield.ctx[error]);
        if (error != io::error::operation_aborted) {
            throw io_error("request timed out");
        }

        return res;
    }

    void
    connection::set_handler(api::Packet::PayloadCase c, const handler& h) {
        handlers_.emplace(std::make_pair(c, h));
    }

    void
    connection::set_stream_cb(int32_t req_id, const handler& cb) {
        open_streams_.emplace(std::make_pair(req_id, cb));
    }

    void 
    connection::write_back(int32_t req_id, api::Packet&& p) {
        p.set_req_id(req_id);
        send(std::move(p));
    }

    void 
    connection::close_stream(int32_t stream_id) {
        open_streams_.erase(stream_id);
    }
}
