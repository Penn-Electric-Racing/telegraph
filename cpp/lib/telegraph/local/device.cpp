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
#include <filesystem>

namespace fs = std::filesystem;

namespace telegraph {

    static params make_device_params(const std::string& port, int baud) {
        std::map<std::string, params, std::less<>> i;
        i["port"] = port;
        i["baud"] = baud;
        return params(std::move(i));
    }

    device::device(io::io_context& ioc, const std::string& name, const std::string& port, int baud)
            : local_context(ioc, name, "device", make_device_params(port, baud), nullptr), 
              write_queue_(), write_buf_(), read_buf_(),
              req_id_(0), reqs_(), adapters_(),
              port_(ioc) {
        boost::system::error_code ec;
        port_.open(port, ec);
        if (ec) throw io_error("unable to open port: " + port);
        port_.set_option(io::serial_port::baud_rate(baud));
    }

    device::~device() {
        port_.close();
    }

    void
    device::init(io::yield_ctx& yield, int timeout_millisec) {
        // start reading (we can't do this in the constructor
        // since there shared_from_this() doesn't work)
        auto sthis = shared_device_this();
        io::dispatch(port_.get_executor(), [sthis] () { sthis->do_reading(0); });

        /*
        io::deadline_timer timer(ioc_, 
            boost::posix_time::milliseconds(1000));
        uint32_t req_id = sthis->req_id_++;
        stream::Packet res;

        sthis->reqs_.emplace(req_id, req(&timer, &res));

        // put in request
        io::dispatch(port_.get_executor(),
                [sthis, req_id] () {
                    stream::Packet p;
                    p.set_req_id(req_id);
                    p.mutable_fetch_tree();
                    sthis->write_packet(std::move(p));
                });

        // need to handle error code
        boost::system::error_code ec;
        timer.async_wait(yield.ctx[ec]);
        reqs_.erase(req_id);
        // if we timed out
        if (ec != io::error::operation_aborted) {
            throw io_error("timed out when connecting to " + params_.at("port").get<std::string>());
        }
        if (!res.has_tree()) {
            throw io_error("got bad response when connecting to " + params_.at("port").get<std::string>());
        }
        // create shared pointer from unpacked tree
        std::shared_ptr<node> tree(node::unpack(res.tree()));
        tree_ = tree;
        if (tree_) {
            tree_->set_owner(shared_device_this());

            // start a "ping" task that periodically sends pings
            auto wp = weak_device_this();
            io::io_context& ioc = ioc_;
            io::spawn(ioc_, [&ioc, wp, timeout_millisec](io::yield_context yield) {
                io::deadline_timer timer{ioc};
                while (true) {
                    timer.expires_from_now(
                        boost::posix_time::milliseconds(timeout_millisec));
                    timer.async_wait(yield);
                    // send a ping
                    {
                        auto sp = wp.lock();
                        if (!sp) break;
                        uint32_t req_id = sp->req_id_++;
                        io::dispatch(sp->port_.get_executor(),
                            [sp, req_id]() {
                                stream::Packet p;
                                p.set_req_id(req_id);
                                p.set_ping(0);
                                sp->write_packet(std::move(p));
                            });
                    }
                }
            });
        }
        */
    }

    void
    device::destroy(io::yield_ctx& ctx) {
        local_context::destroy(ctx);
        port_.close();
        adapters_.clear();
    }

    subscription_ptr 
    device::subscribe(io::yield_ctx& yield, const variable* v,
                        float min_interval, float max_interval, float timeout) {
        // get the adapter for the variable
        node::id id = v->get_id();
        auto it = adapters_.find(id);
        if (it == adapters_.end()) {
            auto wp = std::weak_ptr<device>(shared_device_this());
            auto change = [wp, id](io::yield_ctx& yield, float debounce, 
                            float refresh, float timeout) -> bool {
                // get a shared pointer to the device
                // will be invalid if the device 
                // has been destroyed
                auto sthis = wp.lock();
                if (!sthis) return false;

                io::deadline_timer timer(sthis->ioc_, 
                    boost::posix_time::milliseconds(1000));
                uint32_t req_id = sthis->req_id_++;
                stream::Packet res;

                sthis->reqs_.emplace(req_id, req(&timer, &res));

                // put in the request
                io::dispatch(sthis->port_.get_executor(),
                        [sthis, req_id, id, debounce, refresh, timeout] () {
                            stream::Packet p;
                            p.set_req_id(req_id);
                            stream::Subscribe* s = p.mutable_change_sub();
                            s->set_var_id(id);
                            s->set_sub_timeout((uint32_t) (1000*timeout));
                            s->set_debounce((uint32_t) (1000*debounce));
                            s->set_refresh((uint32_t) (1000*refresh));
                            sthis->write_packet(std::move(p));
                        });
                // wait for response
                boost::system::error_code ec;
                timer.async_wait(yield.ctx[ec]);
                sthis->reqs_.erase(req_id);
                if (ec != io::error::operation_aborted) {
                    // timed out!
                    return false;
                }
                if (!res.success()) {
                    return false;
                }
                return true;
            };
            auto poll = [wp]() {
                auto sthis = wp.lock();
                if (!sthis) return;
                if (!sthis->port_.is_open()) return;
                uint32_t req_id = sthis->req_id_++;
                io::dispatch(sthis->port_.get_executor(),
                    [sthis, req_id] () {
                        stream::Packet p;
                        p.set_req_id(req_id);
                        p.mutable_poll_sub();
                        sthis->write_packet(std::move(p));
                    }
                );
            };
            auto cancel = [wp, id](io::yield_ctx& yield, 
                                        float timeout) -> bool {
                // do the unsubscribe
                auto sthis = wp.lock();
                if (!sthis) return false;
                if (!sthis->port_.is_open()) return true;
                // keep the adapter alive for the duration of this
                // operations
                auto a = sthis->adapters_.at(id);
                sthis->adapters_.erase(id);

                io::deadline_timer timer(sthis->ioc_, 
                    boost::posix_time::milliseconds(1000));
                uint32_t req_id = sthis->req_id_++;
                stream::Packet res;

                sthis->reqs_.emplace(req_id, req(&timer, &res));

                // put in the request
                io::dispatch(sthis->port_.get_executor(),
                        [sthis, req_id, id, timeout] () {
                            stream::Packet p;
                            p.set_req_id(req_id);
                            stream::Cancel * c = p.mutable_cancel_sub();
                            c->set_var_id(id);
                            c->set_cancel_timeout((uint32_t) (1000*timeout));
                            sthis->write_packet(std::move(p));
                        });
                // wait for response
                boost::system::error_code ec;
                timer.async_wait(yield.ctx[ec]);
                sthis->reqs_.erase(req_id);
                if (ec != io::error::operation_aborted) {
                    // timed out!
                    return false;
                }
                if (!res.success()) {
                    return false;
                }
                return true;
            };
            auto a = std::make_shared<adapter<decltype(poll), decltype(change), decltype(cancel)>>(
                                ioc_, v->get_type(), poll, change, cancel);
            adapters_.emplace(id, a);
        }
        return adapters_.at(id)->subscribe(yield, 
                    min_interval, max_interval, timeout);
    }

    value
    device::call(io::yield_ctx& yield, action* a, value arg, float timeout) {
        io::deadline_timer timer(ioc_, 
            boost::posix_time::milliseconds(1000));
        uint32_t req_id = req_id_++;
        stream::Packet res;
        reqs_.emplace(req_id, req(&timer, &res));

        boost::system::error_code ec;
        timer.async_wait(yield.ctx[ec]);
        reqs_.erase(req_id);
        if (ec != io::error::operation_aborted) {
            return value::invalid();
        }
        if (res.event_case() != stream::Packet::kCallCompleted) {
            return value::invalid();
        }
        return value::unpack(res.call_completed());
    }

    void
    device::do_reading(size_t requested) {
        auto shared = shared_device_this();
        std::weak_ptr<device> weak{shared};
        if (requested > 0) {
            io::async_read(port_, read_buf_, boost::asio::transfer_exactly(requested),
                    [weak] (const boost::system::error_code& ec, size_t transferred) {
                        auto s = weak.lock();
                        if (!s) return;
                        s->on_read(ec, transferred);
                    });
        } else {
            // if bytes is 0 we just try and read some
            io::async_read(port_, read_buf_, boost::asio::transfer_at_least(1),
                [weak] (const boost::system::error_code& ec, size_t transferred) {
                    auto s = weak.lock();
                    if (!s) return;
                    s->on_read(ec, transferred);
                });
        }
    }
    void 
    device::on_read(const boost::system::error_code& ec, size_t transferred) {
        if (ec) return; // on error cancel the reading loop

        // calculate the header length
        uint32_t header_val = 0;

        uint8_t byte = 0;
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
            if (header_pos >= read_buf_.size()) break;
            byte = *(io::buffers_begin(read_buf_.data()) + (header_pos++));

            header_val |= (uint32_t) (byte & 0x7F) << bitpos;
            bitpos = (uint_fast8_t)(bitpos + 7);
        } while (byte & 0x80);

        // something went very wrong
        if (bitpos > 32) {
            // clear the read buffer
            read_buf_.consume(read_buf_.size());
        }

        int32_t length = (int32_t) header_val;

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
            auto payload_loc = io::buffers_begin(buf) + header_pos;
            auto end_of_payload_loc = io::buffers_begin(buf) + header_pos + length;
            crc_expected = crc::crc32_buffers(payload_loc, end_of_payload_loc);

            auto checksum_loc = end_of_payload_loc;
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
            packet.ParseFromCodedStream(&input);
            on_read(std::move(packet));
        } else {
            std::cerr << "bad message of length " << length << std::endl;
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
            node::id var_id = (node::id) p.req_id();
            auto it = adapters_.find(var_id);
            if (it == adapters_.end()) return;
            else it->second->update(value::unpack(p.update()));
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
            const std::string_view& name, const std::string_view& type,
            const params& p) {
        int baud = (int) p.at("baud").get<float>();
        const std::string& port = p.at("port").get<std::string>();
        auto s = std::make_shared<device>(ioc, std::string{name}, port, baud);
        s->init(yield, 500);
        return s;
    }

    // the device scanner task that detects new ports
    static std::vector<std::string> fetch_ports() {
        std::vector<std::string> ports;
#if defined(__linux__) && !defined(__ANDROID__)
        try {
            // go list files
            fs::path dir{"/dev"};
            fs::directory_iterator end;
            for (fs::directory_iterator it{dir}; it != end; it++) {
                const fs::path& p = it->path();
                if (p.filename().string().rfind("ttyACM", 0) == 0) {
                    ports.push_back(p.string());
                }
            }
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
#endif
        return ports;
    }

    static params to_params(const std::vector<std::string>& p) {
        std::vector<params> par;
        for (const std::string& s : p) {
            par.push_back(params{s});
        }
        return params{std::move(par)};
    }

    device_scanner::device_scanner(io::io_context& ioc, const std::string_view& name)
            : local_component(ioc, name, "device_scanner", params()),
                    requests_() {
    }

    void
    device_scanner::init() {
        io::io_context& ioc = ioc_;
        // start the device scanner thread...
        auto sp = std::static_pointer_cast<device_scanner>(shared_from_this());
        std::weak_ptr<device_scanner> wp{sp};
        io::spawn(ioc_, [wp, &ioc](io::yield_context yield) {
            io::yield_ctx ctx{yield};
            io::deadline_timer timer{ioc};
            while (true) {
                timer.expires_from_now(boost::posix_time::seconds(2));
                timer.async_wait(yield);
                {
                    auto sp = wp.lock();
                    if (!sp) break;
                    std::vector<std::string> p = fetch_ports();
                    if (p == sp->last_devices_) continue;
                    sp->last_devices_ = p;
                    // broadcast the ports to all parameter streams
                    params ports = to_params(sp->last_devices_);
                    auto it = sp->requests_.begin();
                    while (it != sp->requests_.end()) {
                        auto p = it->second;
                        auto ps = p.lock();
                        if (!ps || ps->is_closed()) {
                            it = sp->requests_.erase(it);
                        } else {
                            ps->write(params{ports});
                            it++;
                        }
                    }
                }
            }
        });

    }

    device_scanner::~device_scanner() {
        for (auto& wp : requests_) {
            auto sp = wp.second.lock();
            if (sp) sp->close();
        }
    }

    params_stream_ptr
    device_scanner::request(io::yield_ctx& yield , const params& p) {
        auto stream = std::make_shared<params_stream>();
        requests_.emplace(stream.get(), std::weak_ptr<params_stream>{stream});

        auto raw = stream.get();
        auto sp = std::static_pointer_cast<device_scanner>(shared_from_this());
        std::weak_ptr<device_scanner> wp{sp};

        // on stream destruction remove stream from
        // our requests if the scanner still exists
        stream->destroyed.add(this, [raw, wp] () {
            auto sp = wp.lock();
            if (!sp) return;
            sp->requests_.erase(raw);
        });

        params ports = to_params(fetch_ports());
        stream->write(std::move(ports));
        return stream;
    }

    local_component_ptr
    device_scanner::create(io::yield_ctx&, io::io_context& ioc,
            const std::string_view& name, const std::string_view& type,
            const params& p) {
        auto sp = std::make_shared<device_scanner>(ioc, name);
        sp->init();
        return sp;
    }
}
