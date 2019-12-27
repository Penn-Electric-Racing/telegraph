#pragma once

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
    template<typename stream>
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

            struct sub_info {
                generic_variable* var;
                generic_subscription sub;
            };
        public:

            // takes a root node and an id-lookup-table
            stream_interface(stream* s,
                    node* root, node* const *id_lookup_table) : 
                stream_(s),
                // stream state
                root_(root), lookup_table_(id_lookup_table),
                send_idx_(0), send_len_(0), send_buffer_(1025), 
                recv_idx_(0), recv_buffer_(1025) { 

                // maximum length we can receive/send is 1025 bytes
            }

            ~stream_interface() {}

            generic_subscription subscribe(generic_variable* v, int32_t min_interval,
                                            int32_t max_interval) override {
                // we don't forward subscription requests over uart from devices
                // since host computers cannot expose nodes to the network
                // this does nothing (i.e don't use the uart as the backing interface)
                return generic_subscription(min_interval, max_interval);
            }

            // called by receive() when we get an event
            void received_event(const telegraph_proto_StreamEvent& event) {
                switch(event.which_event) {
                case telegraph_proto_StreamEvent_children_request_tag: {
                    int32_t parent_id = event.event.children_request.parent_id;

                    telegraph_proto_StreamEvent event;
                    event.which_event = telegraph_proto_StreamEvent_children_tag;
                    event.event.children.parent_id = parent_id;

                    if (parent_id < 0) {
                        // encode a Children event which contains
                        // the root node
                        event.event.children.children.arg = root_;
                        event.event.children.children.funcs.encode = 
                        [] (pb_ostream_t* os, 
                                const pb_field_iter_t* field, void* const* arg) {

                            const node* root = reinterpret_cast<const node*>(*arg);

                            telegraph_proto_Node node;
                            root->pack_def(&node);

                            if(!pb_encode_tag_for_field(os, field))
                                return false;
                            return pb_encode_submessage(os, telegraph_proto_Node_fields, &node);
                        };
                    } else {
                        node* n = lookup_table_[parent_id];
                        event.event.children.children.arg = n;
                        event.event.children.children.funcs.encode = 
                        [] (pb_ostream_t* os, 
                                const pb_field_iter_t* field, void* const* arg) {
                            const node* g = reinterpret_cast<const node*>(*arg);
                            if (!g) return false;
                            return g->pack_children(os, field);
                        };
                    }
                    // spin until all bytes have
                    // been written
                    while (!send(event)) {}
                } break;
                }
            }


            bool send(const telegraph_proto_StreamEvent& event) {
                if (send_idx_ >= send_len_) {
                    pb_ostream_t os = pb_ostream_from_buffer(&send_buffer_[1], 
                                                            send_buffer_.size() - 5);

                    if (!pb_encode(&os, telegraph_proto_StreamEvent_fields, &event)) {
                        return false;
                    }

                    uint8_t header_value = (uint8_t) (os.bytes_written >> 2);
                    send_buffer_[0] = header_value;
                    *reinterpret_cast<uint32_t*>(&send_buffer_[header_value << 2 + 1]) = 
                        fletcher32(reinterpret_cast<uint16_t*>(&recv_buffer_[1]), header_value << 1);

                    send_idx_ = 0;
                    send_len_ = (header_value << 2 + 5);

                    // write out the send buffer
                    send_idx_ += stream_->write(&send_buffer_[send_idx_], send_len_ - send_idx_);
                    return send_idx_ >= send_len_;
                } else {
                    // last time we only got a partial send out!

                    // keep trying to send
                    send_idx_ += stream_->write(&send_buffer_[send_idx_], send_len_ - send_idx_);
                    return send_idx_ >= send_len_;
                }
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
                telegraph_proto_StreamEvent event = telegraph_proto_StreamEvent_init_default;
                if (pb_decode(&str, telegraph_proto_StreamEvent_fields, &event)) {
                    received_event(event);
                }
                recv_idx_ = 0; 
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

            std::unordered_map<int32_t, sub_info> subscriptions_;

            // the tree
            node* root_; // the root node
            node* const *lookup_table_; // id lookup table from id -> node

            size_t send_idx_;
            size_t send_len_;
            std::vector<uint8_t> send_buffer_;
            size_t recv_idx_;
            std::vector<uint8_t> recv_buffer_;
        };
}
