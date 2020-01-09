#include "device.hpp"

#include "../utils/info.hpp"

#include "stream.pb.h"

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include <boost/asio/spawn.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>

namespace telegraph {
    /*
    class io_task : public local_task {
    public:
        io_task(const std::string& name,
                const std::shared_ptr<local_device>& dev,
                int baud) 
            : local_task(name, "device_task", info()),
              baud_(baud), dev_(dev), port_(nullptr), wbuf_(), rbuf_() {
            dev_->task_ = this;
        }
        ~io_task() {
            dev_->task_ = nullptr;
        }

        void write(io::yield_context& yield, const stream::Packet& p) {
            std::ostream output_stream(&wbuf_);
            {
                ::google::protobuf::io::OstreamOutputStream s(&output_stream);
                ::google::protobuf::io::CodedOutputStream cs(&s);
                cs.WriteVarint32((int) p.ByteSizeLong());
                p.SerializeToCodedStream(&cs);
            }
            io::async_write(*port_, wbuf_, yield);
        }

        void read(io::yield_context& yield, stream::Packet* p) {
            uint8_t byte;
            io::async_read(*port_, io::buffer(&byte, 1), yield);
            std::cout << "read " << (int) byte << std::endl;
        }

        void start(io::yield_context yield, 
                   io::io_context& ioc) {
            // setup the stream
            port_ = std::make_unique<io::serial_port>(ioc, dev_->get_port());
            port_->set_option(io::serial_port::baud_rate(baud_));

            stream::Packet p;
            // set fetch tree
            p.mutable_fetch_tree();
            write(yield, p);
            read(yield, &p);
        }

        void stop(io::yield_context yield) {
            // delete the stream
            port_.reset();
        }
    private:
        int baud_;
        std::shared_ptr<local_device> dev_;
        std::unique_ptr<io::serial_port> port_;
        io::streambuf wbuf_;
        io::streambuf rbuf_;
    };*/

    local_device::local_device(const std::string& name, const std::shared_ptr<node>& tree,
                                device_task* task) 
        : local_context(name, "device", info(), tree), task_(nullptr) {}

    device_task::device_task(const std::string& name, 
            const std::string& port, int baud) 
        : local_task(name, "device_worker", info()), dev_() {
    }
}
