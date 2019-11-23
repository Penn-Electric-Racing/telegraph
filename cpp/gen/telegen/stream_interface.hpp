#pragma once

#include "stream.nanopb.h"

namespace telegen {

    /**
     * A stream interface is designed to be used on a bidirectional stream
     * like a uart device
     */
    template<typename stream>
        class stream_interface : public interface {
        public:
            stream_interface(stream* s) : stream_(s) {}
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
                        write();
                    }
                    process_outbound();
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
            // subscriptions originating from the stream
            std::vector<generic_subscription*> subscriptions_;

            // the send buffer, 512 malloc'd bytes 
            // (2 bytes for length, data, 2 bytes for checksum)
            std::unique_ptr<uint8_t[]> send_buffer_;
            size_t idx_;
        };
}
