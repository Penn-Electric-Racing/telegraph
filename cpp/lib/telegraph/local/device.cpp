#include "device.hpp"

#include "../utils/info.hpp"
#include "../utils/io.hpp"

#include "stream.pb.h"

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include <boost/asio.hpp>
#include <variant>

namespace telegraph {
    // device

    static info make_device_info(const std::string& port, int baud) {
        std::map<std::string, info> i;
        i["port"] = port;
        i["baud"] = baud;
        return info(i);
    }


    device::device(io::io_context& ioc, const std::string& name, 
                    const std::string& port, int baud)
            : local_context(ioc, port, "device", make_device_info(port, baud), nullptr), 
              write_queue_(), write_buf_(), read_buf_(),
              sub_reqs_(), call_id_counter_(0), call_reqs_(),
              tree_reqs_(), adapters_(), port_(ioc, port) {

        port_.set_option(io::serial_port::baud_rate(baud));
    }
    device::~device() {}

    void
    device::init(io::yield_ctx& yield) {
        io::deadline_timer timer(ioc_, boost::posix_time::milliseconds(1000));
        std::shared_ptr<node> tree;

        tree_req req = {
            .event = timer, .tree = tree
        };

        tree_reqs_.push_back(req);

        if (tree_reqs_.size() <= 1) {
            io::dispatch(port_.get_executor(),
                    std::bind(&device::start_fetch, shared_device_this()));
        }

        timer.async_wait(yield.ctx);

        // keep processing requests if there are more
        if (tree_reqs_.size() >= 1) {
            io::dispatch(port_.get_executor(),
                    std::bind(&device::start_fetch, shared_device_this()));
        }

        tree_ = tree;
    }

    subscription_ptr 
    device::subscribe(io::yield_ctx& ctx, variable* v,
                        int32_t min_interval, int32_t max_interval) {
        // get the adapter
        return nullptr;
    }

    value
    device::call(io::yield_ctx& yield, action* a, value arg) {
        uint32_t cid = call_id_counter_++;

        io::deadline_timer timer(ioc_,
                boost::posix_time::milliseconds(100));
        value ret; // invalid value by default

        call_req req = {
            .event = timer, .ret = ret
        };
        // put the request in the callback handlers map
        call_reqs_.insert(std::make_pair(cid, req));

        io::dispatch(port_.get_executor(), 
            std::bind(&device::start_call, shared_device_this(), 
                        cid, a->get_id(), arg));

        timer.async_wait(yield.ctx);

        return ret;
    }

    bool 
    device::change_sub(io::yield_ctx& yield, uint32_t var_id, 
                            uint32_t min_interval, uint32_t max_interval) {
        // get the queue for this variable
        auto& q = sub_reqs_[var_id];

        io::deadline_timer timer(ioc_, boost::posix_time::milliseconds(100));
        bool success = false;

        sub_req req = {
            .cancel = false,
            .min_interval = min_interval,
            .max_interval = max_interval,
            .event = timer,
            .success = success
        };

        bool new_queue = q.size() == 0;
        // push a pointer to the 
        // request onto the dequeue
        q.push_back(&req);

        // post process_sub_queue into the worker strand
        if (new_queue) {
            io::dispatch(port_.get_executor(),
                    std::bind(&device::start_next_sub, 
                            shared_device_this(), var_id));
        }

        // wait for the timer to expire
        timer.async_wait(yield.ctx);

        // remove the request
        q.erase(std::find(q.begin(), q.end(), &req));

        // dispatch to process the next request
        if (q.size() > 0) {
            io::dispatch(port_.get_executor(),
                    std::bind(&device::start_next_sub, 
                            shared_device_this(), var_id));
        }

        return success;
    }

    bool 
    device::cancel_sub(io::yield_ctx& yield, uint32_t var_id) {
        // get the queue for this variable
        auto& q = sub_reqs_[var_id];

        // if the request isn't done in 100 milliseconds, die
        io::deadline_timer timer(ioc_, 
                boost::posix_time::milliseconds(100));
        bool success = false;

        sub_req req = {
            .cancel = true,
            .min_interval = 0,
            .max_interval = 0,
            .event = timer,
            .success = success
        };

        bool new_queue = q.size() == 0;
        q.push_back(&req);

        if (new_queue) {
            io::dispatch(port_.get_executor(),
                    std::bind(&device::start_next_sub,
                            shared_device_this(), var_id));
        }

        timer.async_wait(yield.ctx);

        // remove the request
        q.erase(std::find(q.begin(), q.end(), &req));

        if (q.size() > 0) {
            io::dispatch(port_.get_executor(),
                    std::bind(&device::start_next_sub,
                            shared_device_this(), var_id));
        }

        return success;
    }

    void
    device::on_start() {
        do_reading();
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
        read_buf_.commit(transferred);

        // calculate the header length
        uint32_t length = 0;

        uint8_t byte;
        uint8_t byte_pos = 0;
        uint_fast8_t bitpos = 0;
        do {
            // something went very wrong
            if (bitpos >= 32) {
                // clear the read buffer
                read_buf_.consume(read_buf_.size());
                break;
            }
            // next byte
            if (byte_pos > read_buf_.size()) break;
            byte = *(io::buffers_begin(read_buf_.data()) + (byte_pos++));

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
        } else if (read_buf_.size() < byte_pos + length) {
            // read length bytes
            do_reading(byte_pos + length - read_buf_.size());
            return;
        }

        // we have the entire message!
        // consume the header
        read_buf_.consume(byte_pos); 

        // parse the buffer

        // consume the message bytes
        read_buf_.consume(length);

        // call this function again to
        // parse the next message if there is one,
        // and queue another read if there is not
        on_read(ec, 0);
    }

    void 
    device::do_write_next() {
        // grab the front of the write queue
        auto& p = write_queue_.front();
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
        }
        // write_buf_ now has bytes in the input sequence

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
        if (write_queue_.size() > 1) return;
        do_write_next();
    }

    void
    device::on_read(const stream::Packet& p) {
    }

    void
    device::start_call(uint32_t call_id, 
            uint32_t action_id, value arg) {
        stream::Packet p;
        stream::Call* c = p.mutable_call_action();
        c->set_call_id(call_id);
        c->set_action_id(action_id);
        arg.pack(c->mutable_arg());

        write_packet(std::move(p));
    }

    void
    device::start_fetch() {
        // create the fetch packet
        stream::Packet p;
        p.mutable_fetch_tree();
        write_packet(std::move(p));
    }

    void
    device::start_next_sub(uint32_t var_id) {
        if (sub_reqs_.find(var_id) == sub_reqs_.end()) return;
        auto& q = sub_reqs_.at(var_id); // get the queue

        // get the next request from the queue
        auto& e = *q.front();

        // create the stream packet
        stream::Packet p;
        if (e.cancel) {
            p.set_cancel_sub(var_id);
        } else {
            stream::Subscription* s = p.mutable_change_sub();
            s->set_var_id(var_id);
            s->set_min_interval(e.min_interval);
            s->set_max_interval(e.max_interval);
        }
        write_packet(std::move(p));
    }
    
    device_io_task::device_io_task(io::io_context& ioc, 
                    const std::string& name, const std::string& port)
            : local_task(ioc, name, "device", info(port)),
              port_name_(port), dev_() {}

    void
    device_io_task::start(io::yield_ctx& yield, const info& i) {
        int baud = (int) i.number();
        start(yield, baud);
    }

    void
    device_io_task::start(io::yield_ctx& yield, const std::string& name, int baud) {
        if (dev_.lock()) throw io_error("device already open");
        if (!ns_) throw missing_error("not registered in namespace");

        auto dev = std::make_shared<device>(get_executor(), name, port_name_, baud);
        // set up the tree
        dev->init(yield);

        dev->reg(yield, ns_);
        dev_ = dev;
    }

    void
    device_io_task::stop(io::yield_ctx& yield) {
        auto dev = dev_.lock();
        dev_.reset();
        if (dev) {
            dev->destroy(yield);
        }
    }

    void
    device_io_task::destroy(io::yield_ctx& yield) {
        stop(yield);
        local_task::destroy(yield);
    }

    // the device scanner task that detects new ports

    device_scan_task::device_scan_task(io::io_context& ioc, const std::string& name)
                        : local_task(ioc, name, "device_scanner", info()), devices_() {}

    void
    device_scan_task::start(io::yield_ctx& yield, const info& info) {}

    void
    device_scan_task::stop(io::yield_ctx& yield) {}

    void
    device_scan_task::destroy(io::yield_ctx& yield) {
    }
}
