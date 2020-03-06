#include "relay.hpp"

#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>

#include <iostream>
#include <chrono>

#include "../utils/errors.hpp"

#include "api.pb.h"

using tcp = boost::asio::ip::tcp;
namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

static void fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

namespace telegraph {
    relay::listener::listener(io::io_context& ico, 
            tcp::endpoint endpoint, namespace_* local) 
                : local_(local), ioc_(ico), acceptor_(ico) {

        beast::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "open");
            return;
        }
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) {
            fail(ec, "set_option");
            return;
        }
        acceptor_.bind(endpoint, ec);
        if (ec) {
            fail(ec, "bind");
            return;
        }
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) {
            fail(ec, "listen");
            return;
        }
    }

    void
    relay::listener::run() {
        do_accept();
    }

    void
    relay::listener::do_accept() {
        // creates a strand to run on_accept on
        acceptor_.async_accept(net::make_strand(ioc_),
                beast::bind_front_handler(&listener::on_accept, 
                    shared_from_this()));
    }

    void
    relay::listener::on_accept(beast::error_code ec, tcp::socket socket) {
        if (ec) {
            fail(ec, "accept");
            return;
        }
        std::make_shared<client_connection>(local_, std::move(socket))->run();
        // accept the next connection
        do_accept();
    }

    relay::client_connection::client_connection(namespace_* l, boost::asio::ip::tcp::socket&& socket) 
                : connection(true), local_fwd_(*this, l), remote_ns_(*this),
                  ws_(std::move(socket)), buffer_() {}

    void
    relay::client_connection::run() {
        net::dispatch(ws_.get_executor(),
                beast::bind_front_handler(
                    &client_connection::on_run,
                    shared_from_this()));
    }

    void
    relay::client_connection::on_run() {
        ws_.set_option(
                websocket::stream_base::timeout::suggested(
                    beast::role_type::server));

        ws_.async_accept(
            beast::bind_front_handler(
                &client_connection::on_accept,
                shared_from_this()));
    }

    void
    relay::client_connection::on_accept(beast::error_code ec) {
        net::spawn(ws_.get_executor(),
            [this](net::yield_context yield) {
                io::yield_ctx c(yield);
                try {
                    remote_ns_.connect(c);
                } catch (io_error& e) {
                    std::cerr << "failed to initialize client connection" << std::endl;
                }
            });
        // queue a read
        do_read();
    }

    void
    relay::client_connection::do_read() {
        ws_.async_read(buffer_,
                beast::bind_front_handler(
                    &client_connection::on_read,
                    shared_from_this()));
    }

    void
    relay::client_connection::on_read(beast::error_code ec,
                                    size_t bytes_transferred) {
        if (ec == websocket::error::closed) return;
        // now parse the buffer

        // reset the buffer
        buffer_.consume(buffer_.size());
        // queue another read
        do_read();
    }

    void
    relay::client_connection::send(io::yield_ctx& yield, const api::Packet& p) {
        // do the sending
    }


    relay::remote_connection::remote_connection(io::io_context& ctx, 
                        namespace_* local_ns,
                        const std::string_view& host,
                        const std::string_view& port) 
        : connection(false), 
          local_fwd_(*this, local_ns), // will register handlers with connection
          remote_(*this), 
          ws_(boost::asio::make_strand(ctx)), buffer_(),
          host_(host), port_(port) {}

    std::shared_ptr<remote_namespace>
    relay::remote_connection::connect(io::yield_ctx& ctx) {
        io::yield_context& yield = ctx.ctx;
        if (!ws_.is_open()) {
            beast::error_code ec;

            tcp::resolver resolver(ws_.get_executor());
            auto const results = resolver.async_resolve(host_, port_, yield[ec]);
            if (ec) throw io_error("failed to resolve address " + host_ + ":" + port_);

            beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(1));

            beast::get_lowest_layer(ws_).async_connect(results, yield[ec]);
            if (ec) throw io_error("failed to connect to remote " + host_ + ":" + port_);

            // beast has its own timeout system now
            beast::get_lowest_layer(ws_).expires_never();
            ws_.set_option(
                websocket::stream_base::timeout::suggested(
                    beast::role_type::client));

            ws_.set_option(websocket::stream_base::decorator(
                    [](websocket::request_type& req) {
                        req.set(http::field::user_agent,
                            std::string(BOOST_BEAST_VERSION_STRING) + " telegraph-client");
                    }));
            ws_.async_handshake(host_, "/", yield[ec]);
            if (ec) throw io_error("failed handshake with " + host_ + ":" + port_);
            // we are connected!
            // queue a read
            do_read();
        }
        // fetch uuid, setup remote namespace
        // if not already done
        if (!remote_.is_connected()) {
            remote_.connect(ctx);
        }
        return std::shared_ptr<remote_namespace>(shared_from_this(), 
                    &this->remote_);
    }

    void
    relay::remote_connection::send(io::yield_ctx& yield, const api::Packet& p) {
    }

    void
    relay::remote_connection::do_read() {
    }

    void
    relay::remote_connection::on_read(beast::error_code ec, size_t bytes_transferred) {
    }

    relay::relay(io::io_context& ioc, namespace_* ns) : local_(ns), ctx_(ioc) {}

    relay::~relay() {}

    std::shared_ptr<relay::listener>
    relay::bind(tcp::endpoint ep) {
        return std::make_shared<listener>(ctx_, ep, local_);
    }

    std::shared_ptr<relay::remote_connection>
    relay::connect(const std::string_view& host, const std::string_view& port) {
        return std::make_shared<remote_connection>(ctx_, local_, host, port);
    }
}
