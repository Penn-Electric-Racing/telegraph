#include "client.hpp"

#include "../utils/errors.hpp"

#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>

using tcp = boost::asio::ip::tcp;
namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

namespace telegraph {

    client::client(io::io_context& ioc,
            const std::string_view& host,
            const std::string_view& port,
            const std::shared_ptr<namespace_>& local)
        : ioc_(ioc), host_(host), port_(port),
          local_(local) {}

    client::~client() {
        beast::error_code ec;
        if (remote_) remote_->ws_.close(websocket::close_reason(), ec);
    }

    std::shared_ptr<remote_namespace>
    client::connect(io::yield_ctx& cyield) {
        if (!remote_) {
            io::yield_context yield = cyield.ctx;
            beast::error_code ec;

            websocket::stream<beast::tcp_stream> ws(io::make_strand(ioc_));
            tcp::resolver resolver(ws.get_executor());

            auto const results = resolver.async_resolve(host_, port_, yield[ec]);
            if (ec) throw io_error("failed to resolve address " + host_ + ":" + port_);

            beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(1));

            beast::get_lowest_layer(ws).async_connect(results, yield[ec]);
            if (ec) throw io_error("failed to connect to remote " + host_ + ":" + port_);

            // beast has its own timeout system now
            beast::get_lowest_layer(ws).expires_never();
            ws.set_option(
                    websocket::stream_base::timeout::suggested(
                        beast::role_type::client));

            ws.binary(true);

            ws.set_option(websocket::stream_base::decorator(
                        [](websocket::request_type& req) {
                        req.set(http::field::user_agent,
                                std::string(BOOST_BEAST_VERSION_STRING) + " telegraph-client");
                        }));
            ws.async_handshake(host_, "/", yield[ec]);
            if (ec) throw io_error("failed handshake with " + host_ + ":" + port_);

            // create the remote object
            remote_ = std::make_shared<remote>(ioc_, std::move(ws), local_);

            // we are connected!
            // queue a read
            remote_->start_reading();

            // connect!
            std::shared_ptr<remote_namespace> r(remote_, &remote_->remote_ns_);
            std::weak_ptr<remote_namespace> wr(r);

            remote_->remote_ns_.connect(cyield, wr);
        }

        // return aliased shared ptr
        return std::shared_ptr<remote_namespace>(remote_, &remote_->remote_ns_);
    }

    client::remote::remote(io::io_context& ioc,
                           websocket::stream<beast::tcp_stream>&& ws,
                           const std::shared_ptr<namespace_>& local) 
        : connection(ioc, false), 
          local_fwd_(*this, local),
          remote_ns_(ioc, *this),
          ws_(std::move(ws)),
          write_queue_(), write_buf_() {}

    void
    client::remote::start_reading() {
        auto s = shared_from_this();
        io::spawn(ws_.get_executor(), [s] (io::yield_context yield) {
            io::yield_ctx cyield(yield);

            io::streambuf read_buf;
            api::Packet read_packet;

            while (true) {
                beast::error_code ec;
                s->ws_.async_read(read_buf, yield[ec]);

                if (ec && ec != websocket::error::closed
                       && ec != io::error::operation_aborted) {
                    std::cerr << "error " << ec.message() << std::endl;
                }
                if (ec) break;

                {
                    std::istream input_stream(&read_buf);
                    read_packet.ParseFromIstream(&input_stream);
                }
                read_buf.consume(read_buf.size());

                // handle the packet
                s->received(cyield, read_packet);
            }
        });
    }

    void
    client::remote::send(api::Packet&& p) {
        write_queue_.emplace_back(std::move(p));
        if (write_queue_.size() > 1) return;
        do_write_next();
    }

    void
    client::remote::do_write_next() {
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
