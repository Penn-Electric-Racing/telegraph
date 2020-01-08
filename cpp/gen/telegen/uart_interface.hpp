#pragma once

#include "interface.hpp"
#include "nodes.hpp"

#include "stream.nanopb.h"
#include "pb_decode.h"
#include "pb_encode.h"

#include <unordered_map>

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
    template<typename Uart>
        class uart_interface : public interface {
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
        private:
            Uart* const uart_;
            node* const root_; 
            node* const *const lookup_table_;

            std::unique_ptr<uint8_t[]> recv_buf_;
            size_t recv_idx_;
            size_t header_len_;
            size_t payload_len_;
        public:

            // takes a root node and an id-lookup-table
            uart_interface(Uart* u, node* root, node* const *id_lookup_table) : 
                uart_(u), root_(root), lookup_table_(id_lookup_table),
                recv_buf_(new uint8_t[1024]), 
                recv_idx_(0), header_len_(1), payload_len_(0) {}
            ~uart_interface() {}

            std::unique_ptr<subscription> subscribe(variable_base* v, int32_t min_interval,
                                                    int32_t max_interval) override {
                return std::unique_ptr<subscription>(nullptr);
            }

            // called by receive() when we get an event
            void received_packet(const telegraph_stream_Packet& packet) {
                switch(packet.which_event) {
                case telegraph_stream_Packet_fetch_tree_tag: {
                    telegraph_stream_Packet p;
                    p.which_event = telegraph_stream_Packet_tree_tag;
                    root_->pack(&p.event.tree);
                    write_packet(p);
                } break;
                }
            }

            void write_packet(const telegraph_stream_Packet& packet) {
                pb_ostream_t stream;
                stream.state = (void*) uart_;
                stream.max_size = SIZE_MAX;
                stream.callback = [](pb_ostream_t* stream, 
                        const uint8_t* buf, size_t count) {
                    Uart* u = (Uart*) stream->state;
                    u->write(buf, count);
                    return true;
                };
                // write packet with var ints
                pb_encode_ex(&stream, telegraph_stream_Packet_fields, 
                             &packet, PB_ENCODE_DELIMITED);
            }

            void reset_read_buf() {
                recv_idx_ = 0;
                header_len_ = 1;
                payload_len_ = 0;
            }

            // called by the coroutine whenever
            void receive() {
                // read header (1 byte, which is 1/4 the length of the payload)
                if (recv_idx_ < header_len_) {
                    size_t rd = uart_->try_read(&recv_buf_[0], 
                                    header_len_ - recv_idx_);
                    recv_idx_ += rd;
                    if (recv_idx_ < 1) return;
                }
                if (payload_len_ == 0) {
                    uint32_t result = 0;

                    uint8_t byte;
                    uint8_t byte_pos = 0;
                    uint_fast8_t bitpos = 0;
                    do {
                        // something went very wrong
                        if (bitpos >= 32) {
                            reset_read_buf();
                            return;
                        }
                        // next byte
                        byte_pos++;
                        if (byte_pos > header_len_) {
                            header_len_ = byte_pos;
                            return;
                        }
                         byte = recv_buf_[byte_pos];

                        result |= (uint32_t) (byte & 0x7F) << bitpos;
                        bitpos = (uint_fast8_t)(bitpos + 7);
                    } while (byte & 0x80);

                    // something went very wrong
                    if (bitpos > 32) {
                        reset_read_buf();
                        return;
                    }
                    payload_len_ = result;
                }
                size_t expected = header_len_ + payload_len_;
                if (expected > 1024) reset_read_buf();
                if (recv_idx_ < expected) {
                    size_t rd = uart_->try_read(&recv_buf_[0], 
                                                expected - recv_idx_);
                    recv_idx_ += rd;
                    if (recv_idx_ < expected) return;
                }

                // we have all the bytes we need
                telegraph_stream_Packet packet = telegraph_stream_Packet_init_default;
                pb_istream_t stream = pb_istream_from_buffer(&recv_buf_[header_len_], 
                                                            payload_len_);
                if (pb_decode(&stream, telegraph_stream_Packet_fields, &packet)) {
                    received_packet(packet);
                }
                reset_read_buf();
            }

            // coroutine to process messages on this interface
            // should be run in parallel with the co_run for other interfaces
            // and any sensors coroutines
            class co_run : public coroutine {
            private:
                uart_interface* interface_;
            public:
                constexpr co_run(uart_interface* interface) : interface_(interface) {}
                constexpr co_run(uart_interface& interface) : interface_(&interface) {}

                constexpr void operator()() {
                    reset();
                }

                void resume() override {
                    // on resume, check if there are any messages waiting in the uart
                    if (interface_->uart_->has_data()) {
                        interface_->receive();
                    }
                }
            };

            // returns a coroutine that reads from the uart
            // continuously and handles new subscription requests
            co_run run() {
                return co_run(this);
            }
        };
}
