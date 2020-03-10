#ifndef __TELEGRAPH_CLIENT_HPP__
#define __TELEGRAPH_CLIENT_HPP__

#include "../utils/io.hpp"

#include "api.pb.h"

#include "connection.hpp"
#include "namespace.hpp"
#include "forwarder.hpp"

#include <unordered_map>
#include <memory>
#include <deque>

#include <boost/asio/streambuf.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace telegraph {
    class client {
    private:
        class remote : 
            public std::enable_shared_from_this<remote>,
            public connection {
        public:
            forwarder local_fwd_;
            remote_namespace remote_ns_;

            boost::beast::websocket::stream<
                boost::beast::tcp_stream> ws_;

            std::deque<api::Packet> write_queue_;
            io::streambuf write_buf_;

            remote(io::io_context& ioc,
                    boost::beast::websocket::stream<
                    boost::beast::tcp_stream>&& ws, 
                    const std::shared_ptr<namespace_>& local);

            void send(api::Packet&& p) override;

            void start_reading();
            void do_write_next();
        };

        io::io_context& ioc_;
        std::string host_;
        std::string port_;
        std::shared_ptr<namespace_> local_;
        std::shared_ptr<remote> remote_;
    public:
        client(io::io_context& ioc, 
               const std::string_view& host,
               const std::string_view& port, 
               const std::shared_ptr<namespace_>& local);
        ~client();

        std::shared_ptr<remote_namespace> connect(io::yield_ctx& yield);
    };
}

#endif
