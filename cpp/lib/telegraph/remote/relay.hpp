#ifndef __TELEGRAPH_RELAY_HPP__
#define __TELEGRAPH_RELAY_HPP__

#include "connection.hpp"
#include "namespace.hpp"
#include "../utils/io.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>

namespace telegraph {
    class namespace_;

    class relay {
    public:
        // binds to a specific port
        class listener : 
            public std::enable_shared_from_this<listener> {
        public:
            listener(relay* r,
                    boost::asio::io_context& ioc, 
                    boost::asio::ip::tcp::endpoint endpoint);
            void run();
        private:
            void do_accept();
            void on_accept(boost::beast::error_code ec, 
                           boost::asio::ip::tcp::socket socket);

            relay* relay_;
            boost::asio::io_context& ioc_;
            boost::asio::ip::tcp::acceptor acceptor_;
        };

        // handles a client
        class client_connection: 
            public std::enable_shared_from_this<client_connection>,
            public connection {
        public:
            client_connection(relay* r, boost::asio::ip::tcp::socket&& socket);
            void run();

            void send(io::yield_context yield, const api::Packet& p) override;
        private:
            void on_run();
            void do_read();

            void on_accept(boost::beast::error_code ec);
            void on_read(boost::beast::error_code ec, size_t bytes_transferred);

            relay* relay_;
            remote_namespace ns_;

            boost::beast::websocket::stream<
                    boost::beast::tcp_stream> ws_;
            boost::beast::flat_buffer buffer_;
        };

        relay(namespace_* ns);
        ~relay();

        std::shared_ptr<listener> bind(boost::asio::ip::tcp::endpoint ep, 
                                       boost::asio::io_context& ioc);
        friend class client_connection;
    private:
        namespace_* local_;
        // registered namespaces
        std::unordered_map<uuid, namespace_*> reg_;
    };
}

#endif
