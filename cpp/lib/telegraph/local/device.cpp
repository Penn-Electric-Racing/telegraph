#include "device.hpp"

#include "../utils/info.hpp"
#include "../utils/io.hpp"

#include "stream.pb.h"

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include <boost/asio.hpp>
#include <variant>
#include <iostream>
#include <iomanip>
#include <memory>

namespace telegraph {
    // crc utilities
    static const uint32_t crc_table[256] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
    };

    template<typename ConstBuffersIter>
        static uint32_t crc32_buffers(ConstBuffersIter start, ConstBuffersIter end) {
            uint32_t crc = ~0U;
            while (start != end) {
                const uint8_t c = static_cast<uint8_t>(*start);
                crc = crc_table[(crc ^ c) & 0xFF] ^ (crc >> 8);
                start++;
            }
            return crc ^ ~0U;
        }

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
              req_id_(0), reqs_(),
              /*adapters_(),*/ port_(ioc, port) {
        port_.set_option(io::serial_port::baud_rate(baud));

    }
    device::~device() {}

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

    void
    device::destroy(io::yield_ctx& yield) {
        port_.close();
        local_context::destroy(yield);
    }

    subscription_ptr 
    device::subscribe(io::yield_ctx& ctx, variable* v,
                        float min_interval, float max_interval, float timeout) {
        // get the adapter
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
            crc_expected = crc32_buffers(io::buffers_begin(buf), checksum_loc);

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
            uint32_t crc = crc32_buffers(io::buffers_begin(buf), io::buffers_end(buf));
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

    // the device scanner task that detects new ports

    device_scan_task::device_scan_task(io::io_context& ioc, const std::string_view& name)
                        : local_task(ioc, name, "device_scanner", info()) {}

    void
    device_scan_task::start(io::yield_ctx& yield) {}

    void
    device_scan_task::stop(io::yield_ctx& yield) {}

    info_stream_ptr
    device_scan_task::query(io::yield_ctx&yield , const info& info) {
        return nullptr;
    }

    void
    device_scan_task::destroy(io::yield_ctx& yield) {
    }

    local_task_ptr
    device_scan_task::create(io::yield_ctx&, io::io_context& ioc,
            const std::string_view& type, const std::string_view& name,
            const info& params, const sources_map& srcs) {
        return std::make_shared<device_scan_task>(ioc, name);
    }
}
