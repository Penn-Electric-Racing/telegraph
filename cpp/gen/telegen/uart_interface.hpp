#pragma once

#include "interface.hpp"
#include "nodes.hpp"
#include "value.hpp"
#include "promise.hpp"

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
        class uart_interface : public interface, public coroutine {
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
            size_t table_size_;

            // variable id -> subscription object
            std::unordered_map<int32_t, subscription_ptr> subs_;

            std::unique_ptr<uint8_t[]> recv_buf_;
            size_t recv_idx_;
            size_t header_len_;
            size_t payload_len_;
        public:

            // takes a root node and an id-lookup-table
            uart_interface(Uart* u, node* root, 
                    node* const *id_lookup_table, size_t table_size) : 
                uart_(u), root_(root), 
                lookup_table_(id_lookup_table), table_size_(table_size),
                subs_(),
                recv_buf_(new uint8_t[256]), 
                recv_idx_(0), header_len_(1), payload_len_(0) {}
            ~uart_interface() {}

            // nobody can subscribe through here
            promise<subscription_ptr> subscribe(variable_base* v, int32_t min_interval,
                                                    int32_t max_interval) override {
                return promise<subscription_ptr>(promise_status::Rejected);
            }

            promise<value> call(action_base* a, const value& arg) override {
                return promise<value>(promise_status::Rejected);
            }

            void notify_subscribed(int32_t var_id) {
                telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                p.which_event = telegraph_stream_Packet_subscribed_tag;
                p.event.subscribed = var_id;
                write_packet(p);
            }

            void notify_sub_failed(int32_t var_id) {
                telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                p.which_event = telegraph_stream_Packet_sub_failed_tag;
                p.event.sub_failed = var_id;
                write_packet(p);
            }

            void notify_cancelled(int32_t var_id) {
                telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                p.which_event = telegraph_stream_Packet_cancelled_tag;
                p.event.cancelled = var_id;
                write_packet(p);
            }

            void push_update(int32_t var_id, const value& v) {
                telegraph_stream_Packet p =
                    telegraph_stream_Packet_init_default;
                p.which_event = telegraph_stream_Packet_update_tag;
                p.event.update.var_id = var_id;
                v.pack(&p.event.update.val);
            }

            // called by receive() when we get an event
            void received_packet(const telegraph_stream_Packet& packet) {
                switch(packet.which_event) {
                case telegraph_stream_Packet_fetch_tree_tag: {
                    telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                    p.which_event = telegraph_stream_Packet_tree_tag;
                    root_->pack(&p.event.tree);
                    write_packet(p);
                } break;
                case telegraph_stream_Packet_change_sub_tag: {
                    // extract the info
                    int32_t var_id = packet.event.change_sub.var_id;
                    if (var_id >= table_size_) return;
                    variable_base* v = (variable_base*) lookup_table_[var_id];

                    int32_t min_int = packet.event.change_sub.min_interval;
                    int32_t max_int = packet.event.change_sub.max_interval;

                    // the callback for when the operation is complete
                    // NOTE: since we are capturing two variables, requires malloc?
                    // not in a performance-critical pathway, but might be worth looking
                    // into how much using a std::function alternative affects mallocs()

                    if (subs_.find(var_id) != subs_.end()) {
                        auto& s = subs_.at(var_id);
                        auto p = s->change(min_int, max_int);
                        // on change completion
                        p.then([this, var_id] (promise_status s) {
                            if (s == promise_status::Resolved) notify_subscribed(var_id);
                            else notify_sub_failed(var_id);
                        });
                    } else {
                        if (v) {
                            auto p = v->subscribe(min_int, max_int);
                            // on subscribe completion
                            p.then([this, var_id] (promise_status s, subscription_ptr&& sub) {
                                if (s == promise_status::Resolved) {
                                    // put the subscribe in the subs map
                                    // set the handler to push updates
                                    sub->handler([this, var_id] (const value& v) {
                                        push_update(var_id, v);
                                    });
                                    subs_.emplace(var_id, std::move(sub));
                                    notify_subscribed(var_id);
                                } else {
                                    notify_sub_failed(var_id);
                                }
                            });
                        }
                    }
                } break;
                case telegraph_stream_Packet_cancel_sub_tag: {
                    int32_t var_id = packet.event.cancel_sub;
                    if (subs_.find(var_id) != subs_.end()) {
                        auto& s = subs_.at(var_id);
                        auto p = s->cancel();
                        p.then([this, var_id] (promise_status s) {
                            if (s == promise_status::Resolved) {
                                subs_.erase(var_id);
                                notify_cancelled(var_id);
                            }
                        });
                    } else {
                        notify_cancelled(var_id);
                    }
                } break;
                case telegraph_stream_Packet_ping_tag: {
                    telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                    p.which_event = telegraph_stream_Packet_pong_tag;
                    p.event.pong = packet.event.ping;
                    write_packet(p);
                } break;
                default: break;
                }
            }

            void write_packet(const telegraph_stream_Packet& packet) {
                pb_ostream_t stream;
                stream.state = (void*) uart_;
                stream.max_size = SIZE_MAX;
                stream.bytes_written = 0;
                stream.callback = [](pb_ostream_t* stream, 
                        const uint8_t* buf, size_t count) {
                    Uart* u = (Uart*) stream->state;
                    size_t sent = 0;
                    // make sure the entire 
                    // write goes out
                    // TODO: This will block other 
                    // coroutines since this will not yield
                    // but the write buffer should rarely be full
                    // so theoretically this should not be a performance
                    // issue
                    do {
                        sent += u->write(buf, count);
                    } while (sent < count);
                    return true;
                };
                // write packet with var ints
                if (!pb_encode_ex(&stream, telegraph_stream_Packet_fields, 
                                 &packet, PB_ENCODE_DELIMITED)) {
                    // should never be reached!
                    #ifndef NDEBUG
                    while(true) {} 
                    #endif
                }
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
                    size_t rd = uart_->try_read(&recv_buf_[recv_idx_], 
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
                        if (byte_pos > header_len_) {
                            header_len_ = byte_pos;
                            return;
                        }
                        byte = recv_buf_[byte_pos++];

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
                if (expected > 256) reset_read_buf();
                if (recv_idx_ < expected) {
                    size_t rd = uart_->try_read(&recv_buf_[recv_idx_], 
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

            void resume() override {
                if (uart_->has_data()) {
                    receive();
                }
            }
        };
}
