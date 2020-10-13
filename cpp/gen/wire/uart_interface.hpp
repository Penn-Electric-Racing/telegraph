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

namespace wire {
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
            uint8_t recv_prev_;
            bool recv_start_;
            size_t recv_idx_;
        public:

            // takes a root node and an id-lookup-table
            uart_interface(Uart* u, Clock* c, node* root, 
                    node* const *id_lookup_table, size_t table_size, uint32_t timeout = 1000) : 
                uart_(u), clock_(c), root_(root), 
                lookup_table_(id_lookup_table), table_size_(table_size),
                last_time_(0), timeout_(timeout), subs_(),
                recv_buf_(new uint8_t[256]), 
                recv_prev_(0), recv_start_(false), recv_idx_(0) {}
            ~uart_interface() {}

            // nobody can subscribe through here
            promise<subscription_ptr> subscribe(variable_base* v, 
                    interval debounce, interval referesh, interval timeout) override {
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

            void push_update(node::id var_id, const value& v) {
                telegraph_stream_Packet p =
                    telegraph_stream_Packet_init_default;
                p.req_id = var_id;
                p.which_event = telegraph_stream_Packet_update_tag;
                v.pack(&p.event.update);
                write_packet(p);
            }

            // called by receive() when we get an event
            void received_packet(const telegraph_stream_Packet& packet) {
                last_time_ = clock_->millis();
                uint32_t req_id = packet.req_id;
                switch(packet.which_event) {
                case telegraph_stream_Packet_fetch_node_tag: {
                    telegraph_stream_Packet p = telegraph_stream_Packet_init_default;
                    node::id node_id = packet.event.fetch_node;
                    p.req_id = packet.req_id;
                    p.which_event = telegraph_stream_Packet_node_tag;
                    node* n = (node_id >= table_size_) ? root_ : lookup_table_[node_id]; 
                    if (n == nullptr) n = root_;
                    n->pack_condensed(&p.event.node);
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
                    if (packet.event.change_sub.debounce > 
                            std::numeric_limits<interval>::max()) return;
                    if (packet.event.change_sub.refresh > 
                            std::numeric_limits<interval>::max()) return;
                    if (packet.event.change_sub.sub_timeout > 
                            std::numeric_limits<interval>::max()) return;

                    interval min_int = (interval) packet.event.change_sub.debounce;
                    interval max_int = (interval) packet.event.change_sub.refresh;
                    interval timeout = (interval) packet.event.change_sub.sub_timeout;

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
                    if (packet.event.cancel_sub.cancel_timeout > 
                            std::numeric_limits<interval>::max()) return;
                    node::id var_id = packet.event.cancel_sub.var_id;
                    interval timeout = packet.event.cancel_sub.cancel_timeout;
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

                pb_ostream_t raw_stream;
                raw_stream.state = &state;
                raw_stream.max_size = SIZE_MAX;
                raw_stream.bytes_written = 0;
                raw_stream.callback = [](pb_ostream_t* stream, 
                        const uint8_t* buf, size_t count) {
                    stream_state* s = (stream_state*) stream->state;
                    do {
                        size_t written = s->uart->try_write(buf, count);
                        buf += written;
                        count -= written;
                    } while (count > 0);
                    return true;
                };

                pb_ostream_t payload_stream;
                payload_stream.state = &state;
                payload_stream.max_size = SIZE_MAX;
                payload_stream.bytes_written = 0;
                payload_stream.callback = [](pb_ostream_t* stream, 
                        const uint8_t* buf, size_t count) {
                    stream_state* s = (stream_state*) stream->state;
                    for (size_t i = 0; i < count; i++) {
                        util::crc32_next(s->crc, buf[i]);
                        // start, end, escape
                        if (buf[i] == 0x53 || buf[i] == 0x45 || 
                                buf[i] == 0x40) {
                            uint8_t escape = 0x40;
                            while (!s->uart->try_write(&escape, 1));
                        }
                        while (!s->uart->try_write(buf + i, 1));
                    }
                    return true;
                };

                uint8_t val = 0x53;
                while (!uart_->try_write(&val, 1));
                while (!uart_->try_write(&val, 1));

                // write packet with escapes
                if (!pb_encode(&payload_stream, telegraph_stream_Packet_fields,
                                &packet)) {
                    // should never be reached!
                    #ifndef NDEBUG
                    while(true) {}
                    #endif
                }

                // crc is part of the content and is also
                // escaped
                util::crc32_finalize(state.crc);
                uint32_t crc = state.crc;
                pb_write(&payload_stream, 
                    (const uint8_t*) &crc, sizeof(crc));

                val = 0x45;
                while (!uart_->try_write(&val, 1));

                uart_->flush();
            }

            // called by the coroutine whenever
            void receive() {
                if (!recv_start_) {
                    uint8_t val = 0;
                    if (!uart_->try_read(&val, 1)) return;
                    if (val == 0x53 && recv_prev_ == 0x53) {
                        recv_start_ = true;
                        recv_prev_ = 0x00;
                        recv_idx_ = 0;
                    } else {
                        recv_prev_ = val;
                        return;
                    }
                }

                if (recv_start_) {
                    while (true) {
                        if (recv_idx_ >= 255) {
                            // go back to looking for a header
                            recv_prev_ = 0;
                            recv_start_ = false;
                            return;
                        }
                        // decode the payload
                        uint8_t val = 0;
                        if (!uart_->try_read(&val, 1)) return;

                        if (recv_prev_ == 0x40) {
                            recv_buf_[recv_idx_++] = val;
                            recv_prev_ = 0;
                        } else if (val == 0x53) {
                            recv_prev_ = 0x53;
                            recv_start_ = false;
                            return;
                        } else if (val == 0x45) {
                            recv_prev_ = 0;
                            break;
                        } else if (val == 0x40) {
                            recv_prev_ = val;
                        } else {
                            recv_buf_[recv_idx_++] = val;
                        }
                    }
                }

                recv_prev_ = 0;
                recv_start_ = false;
                if (recv_idx_ < 4) return;

                // payload minus the tail
                size_t payload_len = recv_idx_ - 4;

                uint32_t crc = *reinterpret_cast<uint32_t*>(&recv_buf_[recv_idx_ - 4]);
                uint32_t crc_expected = util::crc32_block(&recv_buf_[0], payload_len);
                if (crc != crc_expected) {
                    return;
                }

                telegraph_stream_Packet packet = telegraph_stream_Packet_init_default;
                pb_istream_t stream = pb_istream_from_buffer(
                            &recv_buf_[0], payload_len);
                if (pb_decode(&stream, telegraph_stream_Packet_fields, &packet)) {
                    received_packet(packet);
                }
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
