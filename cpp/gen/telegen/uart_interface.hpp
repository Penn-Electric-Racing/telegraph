#pragma once

namespace telegen {

    template<typename Uart>
        class uart_interface : public interface {
        public:
            uart_interface(Uart* uart) : uart_(uart) {}
            ~uart_interface() {}

            void subscribed(generic_variable* v, generic_subscription* s) override {

            }

            // returns a coroutine that reads from the uart
            // continuously and handles new subscription requests
            co_run run() override {
            }

            class co_run : public coroutine {
            public:
                co_run(uart_interface* interface) {
                }
            private:
            };
        private:
            Uart* uart_;
        };
}
