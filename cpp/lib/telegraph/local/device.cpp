#include "device.hpp"

#include "../utils/io.hpp"

#include "crc.hpp"
#include "stream.pb.h"

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include <boost/asio.hpp>
#include <variant>
#include <iostream>
#include <iomanip>
#include <memory>

namespace telegraph {

    static params make_device_params(const std::string& port, int baud) {
        std::map<std::string, params, std::less<>> i;
        i["port"] = port;
        i["baud"] = baud;
        return params(std::move(i));
    }


    device::device(io::io_context& ioc, const std::string& name, const std::string& port, int baud)
            : local_context(ioc, port, "device", make_device_params(port, baud), nullptr), 
              write_queue_(), write_buf_(), read_buf_(),
              req_id_(0), reqs_(),  // adapters_(),
              port_(ioc) {
        boost::system::error_code ec;
        port_.open(port, ec);
        if (ec) throw io_error("unable to open port: " + port);
        port_.set_option(io::serial_port::baud_rate(baud));
    }

    device::~device() {
        port_.close();
    }

    bool
    device::init(io::yield_ctx& yield) {
        // start reading (we can't do this in the constructor
        // since there shared_from_this() doesn't work)
        auto sthis = shared_device_this();
        io::dispatch(port_.get_executor(), [sthis] () { sthis->do_reading(0); });

        io::deadline_timer timer(ioc_, boost::posix_time::milliseconds(10000));

        uint32_t req_id = req_id_++;
        stream::Packet res;
        reqs_.emplace(req_id, req(&timer, &res));

        io::dispatch(port_.get_executor(),
                [sthis, &timer, &res, &req_id] () {
                    stream::Packet p;
                    p.set_req_id(req_id);
                    p.mutable_fetch_tree();
                    sthis->write_packet(std::move(p));
                });

        // need to handle error code
        boost::system::error_code ec;
        timer.async_wait(yield.ctx[ec]);
        // if we timed out
        if (ec != io::error::operation_aborted ||
                !res.has_tree()) {
            return false;
        }
        // create shared pointer from unpacked tree
        std::shared_ptr<node> tree(node::unpack(res.tree()));
        tree_ = tree;
        reqs_.erase(req_id);

        return tree_ != nullptr;
    }

    subscription_ptr 
    device::subscribe(io::yield_ctx& ctx, const variable* v,
                        float min_interval, float max_interval, float timeout) {
        // get the adapter from the variable!
        return nullptr;
    }

    value
    device::call(io::yield_ctx& yield, action* a, value arg, float timeout) {
        return value();
    }

    void
    device::do_reading(size_t requested) {
        auto shared = shared_device_this();
        if (requested > 0) {
            io::async_read(port_, read_buf_, boost::asio::transfer_exactly(requested),
                    [shared] (const boost::system::error_code& ec, size_t transferred) {
                        shared->on_read(ec, transferred);
                    });
        } else {
            // if bytes is 0 we just try and read some
            io::async_read(port_, read_buf_, boost::asio::transfer_at_least(1),
                [shared] (const boost::system::error_code& ec, size_t transferred) {
                    shared->on_read(ec, transferred);
                });
        }
    }
    void 
    device::on_read(const boost::system::error_code& ec, size_t transferred) {
        if (ec) return; // on error cancel the reading loop

        // calculate the header length
        uint32_t length = 0;

        uint8_t byte;
        uint8_t header_pos = 0;
        uint_fast8_t bitpos = 0;
        do {
            // something went very wrong
            if (bitpos >= 32) {
                // clear the read buffer
                read_buf_.consume(read_buf_.size());
                break;
            }
            // next byte
            if (header_pos > read_buf_.size()) break;
            byte = *(io::buffers_begin(read_buf_.data()) + (header_pos++));

            length |= (uint32_t) (byte & 0x7F) << bitpos;
            bitpos = (uint_fast8_t)(bitpos + 7);
        } while (byte & 0x80);

        // something went very wrong
        if (bitpos > 32) {
            // clear the read buffer
            read_buf_.consume(read_buf_.size());
        }

        if (byte & 0x80) {
            // header is not complete, 
            // try reading any number of bytes
            do_reading(0); 
            return;
        } else {
            size_t expected = header_pos + length + 4;
            if (read_buf_.size() < expected) {
                // read length bytes + 4 bytes for checksum
                do_reading(expected - read_buf_.size());
                return;
            }
        }
        // calculate crc
        uint32_t crc_expected = 0;
        uint32_t crc_actual = 0;
        {
            auto buf = read_buf_.data();
            auto checksum_loc = io::buffers_begin(buf) + header_pos + length;
            crc_expected = crc::crc32_buffers(io::buffers_begin(buf), checksum_loc);

            // hacky but we can only read a byte at a time (might be
            // split across buffers)
            crc_actual |= (uint32_t) ((uint8_t) *(checksum_loc)); 
            checksum_loc++;
            crc_actual |= (uint32_t) ((uint8_t) *(checksum_loc)) << 8; 
            checksum_loc++;
            crc_actual |= (uint32_t) ((uint8_t) *(checksum_loc)) << 16; 
            checksum_loc++;
            crc_actual |= (uint32_t) ((uint8_t) *(checksum_loc)) << 24; 
            checksum_loc++;
        }

        // we have the entire message!
        // consume the header
        read_buf_.consume(header_pos); 

        // parse the buffer if crc valid
        if (crc_actual == crc_expected) {
            // parse the message
            std::istream input_stream(&read_buf_);
			google::protobuf::io::IstreamInputStream iss{&input_stream};
			google::protobuf::io::CodedInputStream input{&iss};
			input.PushLimit(length);

            stream::Packet packet;
            packet.ParseFromIstream(&input_stream);
            on_read(std::move(packet));
        }

        // consume the message bytes and the checksum
        read_buf_.consume(length);
        read_buf_.consume(4);

        // call this function again to
        // parse the next message if there is one,
        // and queue another read if there is not
        on_read(ec, 0);
    }

    void 
    device::do_write_next() {
        // grab the front of the write queue
        auto p = std::move(write_queue_.front());
        write_queue_.pop_front();
        // write into the write_buf_
        {
            std::ostream output_stream(&write_buf_);
            {
                // coded stream must be destructed to flush
                ::google::protobuf::io::OstreamOutputStream s(&output_stream);
                ::google::protobuf::io::CodedOutputStream cs(&s);
                cs.WriteVarint32((int) p.ByteSizeLong());
                p.SerializeToCodedStream(&cs);
            }
            // write crc
            auto buf = write_buf_.data();
            uint32_t crc = crc::crc32_buffers(io::buffers_begin(buf), io::buffers_end(buf));
            output_stream.write(reinterpret_cast<const char*>(&crc), sizeof(crc));
        }
        // write_buf_ now has bytes to be written out in the input sequence

        auto shared = shared_device_this();
        io::async_write(port_, write_buf_.data(), 
            [shared] (const boost::system::error_code& ec, size_t transferred) {
                if (ec) return;
                shared->write_buf_.consume(transferred);
                // if there are more messages, queue another write
                if (shared->write_queue_.size() > 0)
                    shared->do_write_next();
            });
    }

    void
    device::write_packet(stream::Packet&& p) {
        write_queue_.emplace_back(std::move(p));
        // if there is a write chain active
        if (write_queue_.size() > 1) return;
        do_write_next();
    }

    void
    device::on_read(stream::Packet&& p) {
        if (p.has_update()) {
            // updates have var_id in the req_id
            std::cout << "got updated!" << std::endl;
            // spawn a coroutine to deal with the
            // update
        } else {
            // look at the req_id
            uint32_t req_id = p.req_id();
            if (reqs_.find(req_id) != reqs_.end()) {
                auto& r = reqs_.at(req_id);
                if (r.timer) r.timer->cancel();
                if (r.packet) *r.packet = std::move(p);
            }
        }
    }

    local_context_ptr
    device::create(io::yield_ctx& yield, io::io_context& ioc,
            const std::string_view& type, const std::string_view& name,
            const params& p, const sources_map& srcs) {
        int baud = (int) p.at("baud").get<float>();
        const std::string& port = p.at("port").get<std::string>();
        auto s = std::make_shared<device>(ioc, std::string{name}, port, baud);
        s->init(yield);
        return s;
    }

    // the device scanner task that detects new ports

    device_scan_task::device_scan_task(io::io_context& ioc, const std::string_view& name)
                        : local_task(ioc, name, "device_scanner", params()) {}

    void
    device_scan_task::start(io::yield_ctx& yield) {}

    void
    device_scan_task::stop(io::yield_ctx& yield) {}

    params_stream_ptr
    device_scan_task::query(io::yield_ctx&yield , const params& p) {
        return nullptr;
    }

    local_task_ptr
    device_scan_task::create(io::yield_ctx&, io::io_context& ioc,
            const std::string_view& type, const std::string_view& name,
            const params& p, const sources_map& srcs) {
        return std::make_shared<device_scan_task>(ioc, name);
    }
}
