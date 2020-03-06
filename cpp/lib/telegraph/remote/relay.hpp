#ifndef __TELEGRAPH_RELAY_HPP__
#define __TELEGRAPH_RELAY_HPP__

#include "connection.hpp"
#include "forwarder.hpp"
#include "namespace.hpp"
#include "../utils/io.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>
#include <string>
#include <string_view>

namespace telegraph {
    class namespace_;

    class relay {
    public:
        // binds to a specific port
        class listener : 
            public std::enable_shared_from_this<listener> {
        public:
            listener(boost::asio::io_context& ioc,
                     boost::asio::ip::tcp::endpoint endpoint,
                     namespace_* local);
            void run();
        private:
            void do_accept();
            void on_accept(boost::beast::error_code ec, 
                           boost::asio::ip::tcp::socket socket);

            namespace_* local_;
            boost::asio::io_context& ioc_;
            boost::asio::ip::tcp::acceptor acceptor_;
        };

        // handles a client
        class client_connection: 
            public std::enable_shared_from_this<client_connection>,
            public connection {
        private:
            forwarder local_fwd_;
            remote_namespace remote_ns_;

            boost::beast::websocket::stream<
                    boost::beast::tcp_stream> ws_;
            boost::beast::flat_buffer buffer_;
        public:
            client_connection(namespace_* local, boost::asio::ip::tcp::socket&& socket);
            void run();

            void send(io::yield_ctx& yield, const api::Packet& p) override;
        private:
            void on_run();
            void do_read();

            void on_accept(boost::beast::error_code ec);
            void on_read(boost::beast::error_code ec, size_t bytes_transferred);

        };

        // remote connect represents one of our remote connections
        // to a server
        class remote_connection: 
            public std::enable_shared_from_this<remote_connection>,
            public connection {
        private:
            forwarder local_fwd_;
            remote_namespace remote_;

            boost::beast::websocket::stream<
                    boost::beast::tcp_stream> ws_;
            boost::beast::flat_buffer buffer_;

            std::string host_;
            std::string port_;
        public:
            remote_connection(io::io_context& ioc, 
                    namespace_* local_ns, 
                    const std::string_view& host, 
                    const std::string_view& port);

            // will connect to the server and return a shared pointer
            // to the remote namespace. If already connected will just return
            // the shared_ptr

            std::shared_ptr<remote_namespace> connect(io::yield_ctx& yield);

            // for connection, will queue the send
            void send(io::yield_ctx& yield, const api::Packet& p) override;
        private:
            void do_read();
            void on_read(boost::beast::error_code ec, size_t bytes_transferred);
        };

        relay(io::io_context& ioc, namespace_* ns);
        ~relay();

        std::shared_ptr<listener> bind(boost::asio::ip::tcp::endpoint ep);
        std::shared_ptr<remote_connection> connect(const std::string_view& host, 
                                                   const std::string_view& port);

        friend class client_connection;
        friend class remote_connection;
    private:
        namespace_* local_;
        io::io_context& ctx_;
    };
}

#endif
