#pragma once

#include "source.hpp"
#include "nodes.hpp"
#include "value.hpp"
#include "util.hpp"
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
    template<typename Uart, typename Clock>
        class uart_interface : public source, public coroutine {
        private:
            Uart* uart_;
            Clock* clock_;
            node* const root_; 
            node* const *const lookup_table_;
            size_t table_size_;
            
            uint32_t last_time_; // last time we received something
            uint32_t timeout_;

            // variable id -> subscription object
            std::unordered_map<int32_t, subscription_ptr> subs_;

            std::unique_ptr<uint8_t[]> recv_buf_;
            size_t recv_idx_;
            size_t header_len_;
            size_t payload_len_;
        public:

            // takes a root node and an id-lookup-table
            uart_interface(Uart* u, Clock* c, node* root, 
                    node* const *id_lookup_table, size_t table_size, uint32_t timeout) : 
                uart_(u), clock_(c), root_(root), 
                lookup_table_(id_lookup_table), table_size_(table_size),
                last_time_(0), timeout_(timeout), subs_(),
                recv_buf_(new uint8_t[256]), 
                recv_idx_(0), header_len_(1), payload_len_(0) {}
            ~uart_interface() {}

            // nobody can subscribe through here
            promise<subscription_ptr> subscribe(variable_base* v, 
                    interval min_interval, interval max_interval, interval timeout) override {
                return promise<subscription_ptr>(promise_status::Rejected);
            }

            promise<value> call(action_base* a, value arg, interval timeout) override {
                return promise<value>(promise_status::Rejected);
            }

            void notify_success(uint32_t req_id, bool success) {
                telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                p.req_id = req_id;
                p.which_event = telegraph_stream_Packet_success_tag;
                p.event.success = success;
                write_packet(p);
            }

            void notify_cancelled(node::id var_id) {
                telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                p.req_id = 0;
                p.which_event = telegraph_stream_Packet_cancelled_tag;
                p.event.cancelled = var_id;
                write_packet(p);
            }

            void push_update(int32_t var_id, const value& v) {
                telegraph_stream_Packet p =
                    telegraph_stream_Packet_init_default;
                p.req_id = 0;
                p.which_event = telegraph_stream_Packet_update_tag;
                p.event.update.var_id = var_id;
                v.pack(&p.event.update.val);
                write_packet(p);
            }

            // called by receive() when we get an event
            void received_packet(const telegraph_stream_Packet& packet) {
                last_time_ = clock_->millis();
                uint32_t req_id = packet.req_id;
                switch(packet.which_event) {
                case telegraph_stream_Packet_fetch_tree_tag: {
                    telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                    p.which_event = telegraph_stream_Packet_tree_tag;
                    root_->pack(&p.event.tree);
                    write_packet(p);
                } break;
                case telegraph_stream_Packet_change_sub_tag: {
                    // extract the info
                    if (packet.event.change_sub.var_id > 
                            std::numeric_limits<node::id>::max()) return;

                    node::id var_id = packet.event.change_sub.var_id;
                    if (var_id >= table_size_) return;
                    variable_base* v = (variable_base*) lookup_table_[var_id];
                    if (!v) return;

                    // check for overflows in the intervals
                    if (packet.event.change_sub.min_interval > 
                            std::numeric_limits<interval>::max()) return;
                    if (packet.event.change_sub.max_interval > 
                            std::numeric_limits<interval>::max()) return;
                    if (packet.event.change_sub.timeout > 
                            std::numeric_limits<interval>::max()) return;

                    interval min_int = (interval) packet.event.change_sub.min_interval;
                    interval max_int = (interval) packet.event.change_sub.max_interval;
                    interval timeout = (interval) packet.event.change_sub.timeout;

                    // the callback for when the operation is complete
                    // NOTE: since we are capturing two variables, requires malloc?
                    // not in a performance-critical pathway, but might be worth looking
                    // into how much using a std::function alternative affects mallocs()

                    if (subs_.find(var_id) != subs_.end()) {
                        auto& s = subs_.at(var_id);
                        auto p = s->change(min_int, max_int, timeout);
                        // on change completion
                        p.then([this, req_id] (promise_status s) {
                            notify_success(req_id, s == promise_status::Resolved);
                        });
                    } else {
                        if (v) {
                            auto p = v->subscribe(min_int, max_int, timeout);
                            // on subscribe completion
                            p.then([this, req_id, var_id] (promise_status s, subscription_ptr&& sub) {
                                if (s == promise_status::Resolved) {
                                    // put the subscribe in the subs map
                                    // set the handler to push updates
                                    sub->handler([this, var_id] (const value& v) {
                                        push_update(var_id, v);
                                    });
                                    sub->cancel_handler([this, var_id] () {
                                        notify_cancelled(var_id);
                                    });
                                    subs_.emplace(var_id, std::move(sub));
                                }
                                notify_success(req_id, s == promise_status::Resolved);
                            });
                        }
                    }
                } break;
                case telegraph_stream_Packet_cancel_sub_tag: {
                    if (packet.event.cancel_sub.var_id > 
                            std::numeric_limits<node::id>::max()) return;
                    if (packet.event.cancel_sub.timeout > 
                            std::numeric_limits<interval>::max()) return;
                    node::id var_id = packet.event.cancel_sub.var_id;
                    interval timeout = packet.event.cancel_sub.timeout;
                    if (subs_.find(var_id) != subs_.end()) {
                        // erase the variable from the subscriptions
                        // the cancel handler should invoke on
                        // subscription destruction
                        subs_.erase(var_id);
                    }
                } break;
                case telegraph_stream_Packet_ping_tag: {
                    telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                    p.which_event = telegraph_stream_Packet_pong_tag;
                    p.event.pong = subs_.size(); // send back number of active subscriptions
                    write_packet(p);
                } break;
                default: break;
                }
            }

            void write_packet(const telegraph_stream_Packet& packet) {
                struct stream_state {
                    Uart* uart;
                    uint32_t crc;
                };

                stream_state state;
                state.uart = uart_;
                util::crc32_start(state.crc);

                pb_ostream_t stream;
                stream.state = &state;
                stream.max_size = SIZE_MAX;
                stream.bytes_written = 0;
                stream.callback = [](pb_ostream_t* stream, 
                        const uint8_t* buf, size_t count) {
                    stream_state* s = (stream_state*) stream->state;
                    for (size_t i = 0; i < count; i++) {
                        util::crc32_next(s->crc, buf[i]);
                    }
                    // make sure the entire 
                    // write goes out
                    // TODO: This will block other 
                    // coroutines since this will not yield
                    // but the write buffer should rarely be full
                    // so theoretically this should not be a performance
                    // issue. If we had a spin-based clock to lock the uart
                    // interface and could "resume" the encode then we could
                    // yield control here. Not sure if worth the effort though
                    do {
                        count -= s->uart->try_write(buf, count);
                    } while (count > 0);
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
                util::crc32_finalize(state.crc);
                // write the crc
                size_t count = sizeof(uint32_t);
                do {
                    count -= uart_->try_write((const uint8_t*) &state.crc, count);
                } while (count > 0);
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
                size_t expected = header_len_ + payload_len_ + 4;
                if (expected > 256) reset_read_buf();
                if (recv_idx_ < expected) {
                    size_t rd = uart_->try_read(&recv_buf_[recv_idx_], 
                                                expected - recv_idx_);
                    recv_idx_ += rd;
                    if (recv_idx_ < expected) return;
                }

                // we have all the bytes we need
                uint32_t crc = *reinterpret_cast<uint32_t*>(&recv_buf_[header_len_ + payload_len_]);
                uint32_t crc_expected = util::crc32_block(&recv_buf_[0], header_len_ + payload_len_);
                if (crc != crc_expected) {
                    // ERROR WITH THE CHECKSUMS!
                    reset_read_buf();
                    return;
                }

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
                if (last_time_ > 0 &&
                        clock_->millis() > last_time_ + timeout_) {
                    // clear the subscriptions
                    subs_.clear();
                    last_time_ = 0;
                }
            }
        };
}
