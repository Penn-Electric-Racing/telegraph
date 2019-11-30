#pragma once

#include "stream.nanopb.h"

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
                stream_(s), clock_(c) {}
            ~stream_interface() {}

            void subscribed(generic_variable* v, 
                            generic_subscription* s) override {
                // we don't forward subscription requests over uart from devices
                // since host computers cannot expose nodes to the network (currently)
                // so this does nothing
            }

            void receive() {
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

            size_t last_time_;

            // the next min_time to sleep until (microseconds)
            size_t next_time_;

            // the write-out buffer, also contains the 
            size_t send_idx_;
            std::vector<uint8_t> send_buffer_;

            size_t recv_idx_;
            std::vector<uint8_t> recv_buffer_;
        };
}
