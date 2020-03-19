#include "server.hpp"

#include "../utils/errors.hpp"

#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>

using tcp = boost::asio::ip::tcp;
namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

namespace telegraph {
    server::server(io::io_context& ioc, tcp::endpoint ep, 
            const std::shared_ptr<namespace_>& local) 
        : ioc_(ioc), ep_(ep),
          local_(local) {}

    void
    server::run(io::yield_ctx& cyield) {
        // unwrap
        io::yield_context yield = cyield.ctx;

        beast::error_code ec;

        tcp::acceptor acceptor(ioc_);
        acceptor.open(ep_.protocol(), ec);
        if (ec) throw io_error("failed to open server socket");

        acceptor.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) throw io_error("failed to set server socket options");

        acceptor.bind(ep_, ec);
        if (ec) throw io_error("failed to bind to server socket");

        acceptor.listen(net::socket_base::max_listen_connections, ec);
        if (ec) throw io_error("failed to listen to server socket");

        while (true) {
            // create a socket with its own strand
            tcp::socket socket(net::make_strand(ioc_));
            acceptor.async_accept(socket, yield[ec]);
            // we have a socket!
            // create a connection
            std::shared_ptr<remote> conn = std::make_shared<remote>(ioc_, std::move(socket), local_);
            conn->do_accept(); // will start the connection handling
        }
    }

    server::remote::remote(io::io_context& ioc,
            tcp::socket&& socket, 
            const std::shared_ptr<namespace_>& local) 
        : connection(ioc, true), local_fwd_(*this, local),
          remote_ns_(ioc, *this),
          ws_(std::move(socket)) {}

    void
    server::remote::send(api::Packet&& p) {
        write_queue_.emplace_back(std::move(p));
        if (write_queue_.size() > 1) return;
        do_write_next();
    }

    void
    server::remote::do_accept() {
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::server));
        ws_.binary(true);
        ws_.async_accept(
            beast::bind_front_handler(
                &remote::on_accept,
                shared_from_this()));
    }

    void
    server::remote::on_accept(beast::error_code ec) {
        if (ec) {
            std::cerr << "error accepting client connection" << std::endl;
        } else {
            start_reading();
        }
    }

    void
    server::remote::start_reading() {
        auto s = shared_from_this();
        io::spawn(ws_.get_executor(), [s] (io::yield_context yield) {
            io::yield_ctx cyield(yield);

            io::streambuf read_buf;
            api::Packet read_packet;

            while (true) {
                beast::error_code ec;
                s->ws_.async_read(read_buf, yield[ec]);

                if (ec && ec != websocket::error::closed 
                       && ec != io::error::operation_aborted
                       && ec != io::error::eof) {
                    std::cerr << "error: " << ec.message() << std::endl;
                }
                if (ec) break;

                {
                    std::istream input_stream(&read_buf);
                    read_packet.ParseFromIstream(&input_stream);
                }
                read_buf.consume(read_buf.size());

                // handle the packets synchronously
                // for now (TODO: Switch to parallel handling)
                s->received(cyield, read_packet);
            }
        });
    }

    void
    server::remote::do_write_next() {
        if (write_queue_.size() == 0) return;
        auto p = std::move(write_queue_.front());
        write_queue_.pop_front();
        {
            std::ostream output_stream(&write_buf_);
            p.SerializeToOstream(&output_stream);
            output_stream.flush();
        }

        auto shared = shared_from_this();
        ws_.async_write(write_buf_.data(),
                [shared] (const boost::system::error_code& ec, size_t transferred) {
                    shared->write_buf_.consume(transferred);
                    if (ec) return;
                    shared->do_write_next();
                });
    }
}
