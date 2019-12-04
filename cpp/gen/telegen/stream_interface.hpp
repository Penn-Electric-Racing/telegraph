#pragma once

#include "stream.nanopb.h"
#include "pb_decode.h"

namespace telegen {

    /**
     * A stream interface is designed to be used on a bidirectional stream
     * like a uart device
     *
     * This is templated on the i/o stream class type and the timer type
     * so that it can be reused for different platforms, each of which can
     * implement their own stream/timer type 
     * (and so maximize performance by templating it)
     */
    template<typename stream, typename clock>
        class stream_interface : public interface {
        private:
            static uint32_t fletcher32(const uint16_t *data, size_t len) {
                uint32_t c0, c1;
                unsigned int i;

                for (c0 = c1 = 0; len >= 360; len -= 360) {
                    for (i = 0; i < 360; ++i) {
                        c0 = c0 + *data++;
                        c1 = c1 + c0;
                    }
                    c0 = c0 % 65535;
                    c1 = c1 % 65535;
                }
                for (i = 0; i < len; ++i) {
                    c0 = c0 + *data++;
                    c1 = c1 + c0;
                }
                c0 = c0 % 65535;
                c1 = c1 % 65535;
                return (c1 << 16 | c0);
            }
        public:
            // a variable packing
            struct packing {
            };

            // a response to a subscription request
            struct sub_response {

            };

            // a response to a children request
            struct children_req {
            };

            struct action_complete {
            };

            stream_interface(stream* s, clock* c) : 
                stream_(s), clock_(c), 
                // stream state
                packings_(), sub_responses_(), children_requests_(),
                last_time_(0), next_time_(0), 
                send_idx_(0), send_buffer_(1025), recv_idx_(0), 
                recv_buffer_(1025) { // maximum length we can receive/send is 1025 bytes
            }
            ~stream_interface() {}

            void subscribed(generic_variable* v, 
                            generic_subscription* s) override {
                // we don't forward subscription requests over uart from devices
                // since host computers cannot expose nodes to the network (currently)
                // so this does nothing
            }


            // called by the coroutine whenever
            void receive() {
                // read header (1 byte, which is 1/4 the length of the payload)
                if (recv_idx_ < 1) {
                    size_t rd = stream_->try_read(&recv_buffer_[0], 1);
                    recv_idx_ += rd;
                    if (recv_idx_ < 1) return;
                }
                // read payload (4*header value + 4 for checksum number of bytes)
                uint16_t packet_size = ((uint16_t) *reinterpret_cast<uint8_t*>(&recv_buffer_[0])) << 2;
                uint16_t total_size = packet_size + 5; // 1 byte header + packet_size + 4 byte checksum
                if (recv_idx_ < total_size) {
                    size_t rd = stream_->try_read(&recv_buffer_[recv_idx_], 
                                                  total_size - recv_idx_);
                    recv_idx_ += rd;
                    if (recv_idx_ < total_size) return;
                }
                // checksum check
                uint32_t actual_checksum = *reinterpret_cast<uint32_t*>(&recv_buffer_[packet_size + 1]);
                uint32_t expected_checksum = 
                    fletcher32(reinterpret_cast<uint16_t*>(&recv_buffer_[1]), packet_size >> 1);
                if (actual_checksum != expected_checksum) {
                    // reset the receive index and silently drop the packet
                    recv_idx_ = 0; 
                    return;
                }

                // decode the payload using nanopb
                pb_istream_t str = pb_istream_from_buffer(&recv_buffer_[1], packet_size);

                // set the decoding callback
                telegraph_proto_StreamPacket packet;
                packet.events.arg = this;
                packet.events.funcs.decode = 
                    [](pb_istream_t* s, const pb_field_t* f, void** arg) -> bool {
                        return false;
                    };

                // decode!
                pb_decode(&str, telegraph_proto_StreamPacket_fields, &packet);
                recv_idx_ = 0; 
            }

            // writes out the send buffer
            void write() {
            }

            // coroutine to process messages on this interface
            // should be run in parallel with the co_run for other interfaces
            // and any sensors coroutines
            class co_run : public coroutine {
            public:
                constexpr co_run(stream_interface* interface) : interface_(interface) {}
                constexpr co_run(stream_interface& interface) : interface_(&interface) {}

                constexpr void operator()() {
                    reset();
                }

                void resume() override {
                    // on resume, check if there are any messages waiting in the uart
                    if (interface_->stream_->has_data()) {
                        interface_->receive();
                    }
                    // if we have to respond to any subscriptions
                    if (interface_->sub_responses_.size() > 0 ||
                            interface_->children_requests_.size() > 0 ||
                            interface_->clock_->current_microseconds() >= interface_->next_time_) {
                        interface_->write();
                    }
                }

            private:
                stream_interface* interface_;
            };

            // returns a coroutine that reads from the uart
            // continuously and handles new subscription requests
            co_run run() {
                return co_run(this);
            }

        private:
            stream* stream_;
            clock* clock_;

            // contains the packing information for all the
            // subscribed-to variables, indexed by the packing id
            std::vector<packing> packings_;

            std::vector<sub_response> sub_responses_;
            // children requests to be filled on
            std::vector<children_req> children_requests_;

            uint32_t last_time_;

            // the next min_time to sleep until (microseconds)
            size_t next_time_;

            // the write-out buffer, also contains the 
            // headers/checksums

            // the indices contain the next bytes
            size_t send_idx_;
            std::vector<uint8_t> send_buffer_;

            size_t recv_idx_;
            std::vector<uint8_t> recv_buffer_;
        };
}
