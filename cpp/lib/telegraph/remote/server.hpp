#ifndef __TELEGRAPH_SERVER_HPP__
#define __TELEGRAPH_SERVER_HPP__

#include "../utils/io.hpp"

#include "api.pb.h"

#include "connection.hpp"
#include "forwarder.hpp"
#include "../common/namespace.hpp"

#include <unordered_map>
#include <memory>
#include <deque>

#include <boost/asio/streambuf.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>


namespace telegraph {
    class server {
    private:
        io::io_context& ioc_;
        boost::asio::ip::tcp::endpoint ep_;
        std::shared_ptr<namespace_> local_;
    public:
        class remote : 
            public std::enable_shared_from_this<remote>,
            public connection {
        private:
            forwarder local_fwd_;
            boost::beast::websocket::stream<
                boost::beast::tcp_stream> ws_;

            std::deque<api::Packet> write_queue_;
            io::streambuf write_buf_;
        public:
            remote(io::io_context& ioc,
                   boost::asio::ip::tcp::socket&& socket, 
                   const std::shared_ptr<namespace_>& local);

            void send(api::Packet&& p) override;

            void do_accept();
        private:
            void on_accept(boost::beast::error_code ec);

            void start_reading();
            void do_write_next();
        };

        server(io::io_context& ioc, 
            boost::asio::ip::tcp::endpoint ep,
            const std::shared_ptr<namespace_>& local);

        // will handle exceptions
        void run(io::yield_ctx& yield);
    };
}

#endif
